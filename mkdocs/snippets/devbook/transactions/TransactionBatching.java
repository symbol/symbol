//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.HexFormat;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class TransactionBatching {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	public static void main(final String[] args) {
		try {
			new TransactionBatching().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		final String signerPrivateKey = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(signerPrivateKey));

		final Address signerAddress = facade.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.printf("Signer public key: %s%n",
			signerKeyPair.getPublicKey());
		System.out.printf("Signer address: %s%n", signerAddress);

		final String recipient1String = System.getenv().getOrDefault(
			"RECIPIENT_1", "TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI");
		final String recipient2String = System.getenv().getOrDefault(
			"RECIPIENT_2", "TCD4NC5VIE2EEB3BCV5JRLBNJXYDW5Q5JK547MI");
		final Address recipient1 = new Address(recipient1String);
		final Address recipient2 = new Address(recipient2String);
		final String recipient1Hex = HexFormat.of().formatHex(
			recipient1.bytes()).toUpperCase();
		final String recipient2Hex = HexFormat.of().formatHex(
			recipient2.bytes()).toUpperCase();
		System.out.printf("Recipient 1: %s (%s)%n",
			recipient1String, recipient1Hex);
		System.out.printf("Recipient 2: %s (%s)%n",
			recipient2String, recipient2Hex);
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
		// Embedded tx 1: Send 5 XYM to Recipient 1 [>step-3]
		final long xymMosaicId = IdGenerator.generateMosaicAliasId(
			"symbol.xym");
		final EmbeddedTransaction embeddedTx1 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					recipient1,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(xymMosaicId),
						new Amount(5_000_000))), // 5 XYM
					null),
				signerKeyPair.getPublicKey());

		// Embedded tx 2: Send 3 XYM to Recipient 2
		final EmbeddedTransaction embeddedTx2 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					recipient2,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(xymMosaicId),
						new Amount(3_000_000))), // 3 XYM
					null),
				signerKeyPair.getPublicKey());
		// [<step-3]
		// Build the aggregate transaction [>step-4]
		final List<EmbeddedTransaction> embeddedTransactions =
			List.of(embeddedTx1, embeddedTx2);
		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						embeddedTransactions),
					embeddedTransactions,
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		System.out.println("Built aggregate transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));
		// [<step-4]
		// Sign transaction and generate final payload [>step-5]
		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, transaction);
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(transaction, signature);

		// Announce the transaction
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
		// [<step-5]
		// Wait for confirmation [>step-6]
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		final String statusPath = "/transactionStatus/" + transactionHash;
			System.out.printf(
				"Waiting for confirmation from %s%n", statusPath);

		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			try {
				final HttpRequest statusRequest = HttpRequest.newBuilder(
					URI.create(nodeUrl + statusPath)).GET().build();
				final HttpResponse<String> response = HTTP_CLIENT.send(
					statusRequest, BodyHandlers.ofString());
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
			} catch (final IOException ex) {
				System.out.printf(
					"  Transaction status: unknown | Cause: %s%n",
					ex.getMessage());
			}
		} // [<step-6]
	}
}
