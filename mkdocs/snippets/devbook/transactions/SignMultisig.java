//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class SignMultisig {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = "https://reference.symboltest.net:3001";

	private final SymbolFacade facade = new SymbolFacade("testnet");

	public static void main(final String[] args) {
		try {
			new SignMultisig().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		final String multisigPrivateKey = System.getenv().getOrDefault(
			"MULTISIG_PRIVATE_KEY", "%064X".formatted(1));

		final KeyPair multisigKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(multisigPrivateKey));
		System.out.printf("Multisig public key: %s%n",
			multisigKeyPair.getPublicKey());
		final String cosignatory0PrivateKey = System.getenv().getOrDefault(
			"COSIGNATORY0_PRIVATE_KEY", "%064X".formatted(2));
		final KeyPair cosignatoryKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(cosignatory0PrivateKey));
		System.out.printf("Cosignatory public key: %s%n",
			cosignatoryKeyPair.getPublicKey());
		// [<step-1]

		// Fetch recommended fees [>step-2]
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final HttpRequest feeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + feePath)).GET().build();
		final HttpResponse<String> feeResponse = HTTP_CLIENT.send(
			feeRequest, BodyHandlers.ofString());
		final JsonNode feeJSON = JSON_MAPPER.readTree(feeResponse.body());
		final long medianMultiplier =
			feeJSON.get("medianFeeMultiplier").asLong();
		final long minimumMultiplier =
			feeJSON.get("minFeeMultiplier").asLong();
		final long feeMultiplier = Math.max(
			medianMultiplier, minimumMultiplier);
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		// [<step-2]
		// Build the embedded transfer transaction [>step-3]
		final EmbeddedTransaction transferTransaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					facade.network.publicKeyToAddress(
						multisigKeyPair.getPublicKey()),
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
						new Amount(1_000_000))), // 1 XYM
					null),
				multisigKeyPair.getPublicKey());
		// [<step-3]
		// Build the wrapper aggregate transaction [>step-4]
		final List<EmbeddedTransaction> embeddedTransactions =
			List.of(transferTransaction);
		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						embeddedTransactions),
					embeddedTransactions,
					null),
				cosignatoryKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-4]
		// [>step-5]
		// Sign the aggregate using the cosignatory's signature
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(
				transaction,
				facade.signTransaction(
					cosignatoryKeyPair, transaction));
		System.out.println("Built transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));
		// [<step-5]
		// Announce the transaction [>step-6]
		final String announcePath = "/transactions";
		System.out.printf("Announcing transaction to %s%n", announcePath);
		final HttpRequest announceRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + announcePath))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(jsonPayload))
			.build();
		final HttpResponse<String> announceResponse = HTTP_CLIENT.send(
			announceRequest, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", announceResponse.body());

		// Wait for confirmation
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		final String statusPath = "/transactionStatus/" + transactionHash;
			System.out.printf(
				"Waiting for confirmation from %s%n", statusPath);

		for (int attempt = 1; 60 >= attempt; ++attempt) {
			final HttpRequest statusRequest = HttpRequest.newBuilder(
				URI.create(nodeUrl + statusPath)).GET().build();
			final HttpResponse<String> response = HTTP_CLIENT.send(
				statusRequest, BodyHandlers.ofString());

			if (response.statusCode() / 100 == 2) {
				final JsonNode status = JSON_MAPPER.readTree(
					response.body());
				System.out.printf("  Transaction status: %s%n",
					status.get("group").asText());
				if ("confirmed".equals(status.get("group").asText())) {
					System.out.printf(
						"Transaction confirmed in %d seconds%n",
						attempt);
					break;
				}
				if ("failed".equals(status.get("group").asText())) {
					System.out.printf("Transaction failed: %s%n",
						status.get("code").asText());
					break;
				}
			} else {
				System.out.printf(
					"  Transaction status: unknown | Cause: %d%n",
					response.statusCode());
			}
			Thread.sleep(1000);
			if (60 == attempt)
				System.out.println("Confirmation took too long.");
		} // [<step-6]
	}
}
