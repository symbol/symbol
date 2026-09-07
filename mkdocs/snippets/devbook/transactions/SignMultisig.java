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

	private void announceTransaction(
		final String payload,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Announcing %s to /transactions%n", label);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + "/transactions"))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", response.body());
	}

	private void waitForConfirmation(
		final String transactionHash,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Waiting for %s confirmation...%n", label);
		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			final String statusPath =
				"/transactionStatus/" + transactionHash;
			final HttpRequest statusRequest = HttpRequest.newBuilder(
				URI.create(nodeUrl + statusPath)).GET().build();
			final HttpResponse<String> statusResponse = HTTP_CLIENT
				.send(statusRequest, BodyHandlers.ofString());
			if (404 == statusResponse.statusCode()) {
				System.out.println("  Transaction status: unknown");
				continue;
			}
			if (2 != statusResponse.statusCode() / 100)
				throw new IOException(
					"HTTP " + statusResponse.statusCode());

			final JsonNode status =
				JSON_MAPPER.readTree(statusResponse.body());
			final String group = status.get("group").asText();
			System.out.printf("  Transaction status: %s%n", group);
			if ("confirmed".equals(group)) {
				System.out.printf("%s confirmed in %d seconds%n",
					label, attempt);
				return;
			}
			if ("failed".equals(group))
				throw new IOException(String.format("%s failed: %s",
					label, status.get("code").asText()));
		}
		throw new IOException(String.format(
			"%s not confirmed after 60 seconds", label));
	}

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
			cosignatoryKeyPair.getPublicKey()); // [<step-1]


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
				2 * 60 * 60); // [<step-4]

		// Sign the aggregate using the cosignatory's signature [>step-5]
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(
				transaction,
				facade.signTransaction(
					cosignatoryKeyPair, transaction));
		System.out.println("Built transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson())); // [<step-5]

		// Announce the transaction [>step-6]
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf("Transaction hash: %s%n", transactionHash);
		announceTransaction(jsonPayload, "transaction");

		// Wait for confirmation
		waitForConfirmation(transactionHash, "transaction"); // [<step-6]
	}
}
