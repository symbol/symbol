//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.FeeCalculator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.Metadata;
import org.symbol.sdk.symbol.NetworkTimestamp;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.models.*;

final class AccountMetadata {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

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
			try {
				final String statusPath =
					"/transactionStatus/" + transactionHash;
				final HttpRequest statusRequest = HttpRequest.newBuilder(
					URI.create(nodeUrl + statusPath)).GET().build();
				final HttpResponse<String> statusResponse = HTTP_CLIENT
					.send(statusRequest, BodyHandlers.ofString());
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
			} catch (final IOException ex) {
				if (ex.getMessage().contains("failed"))
					throw ex;

				System.out.println("  Transaction status: unknown");
			}
		}
		throw new IOException(String.format(
			"%s not confirmed after 60 seconds", label));
	}

	public static void main(final String[] args) {
		try {
			new AccountMetadata().run();
		} catch (final Exception ex) {
			System.out.println(ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		final String privateKeyString = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(privateKeyString));

		final Address signerAddress = facade.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.printf("Signer address: %s%n", signerAddress);
		// [<step-1]
		// Fetch current network time [>step-2]
		final String timePath = "/node/time";
		System.out.printf(
			"Fetching current network time from %s%n", timePath);
		final HttpRequest timeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + timePath)).GET().build();
		final HttpResponse<String> timeResponse = HTTP_CLIENT.send(
			timeRequest, BodyHandlers.ofString());
		final JsonNode timeJson = JSON_MAPPER.readTree(
			timeResponse.body());
		final NetworkTimestamp timestamp = new NetworkTimestamp(
			timeJson.get("communicationTimestamps")
				.get("receiveTimestamp").asLong());
		System.out.printf("  Network time: %dms since nemesis%n",
			timestamp.timestamp);

		// Fetch recommended fees
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final HttpRequest feeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + feePath)).GET().build();
		final HttpResponse<String> feeResponse = HTTP_CLIENT.send(
			feeRequest, BodyHandlers.ofString());
		final JsonNode feeJson = JSON_MAPPER.readTree(feeResponse.body());
		final long feeMultiplier = Math.max(
			feeJson.get("medianFeeMultiplier").asLong(),
			feeJson.get("minFeeMultiplier").asLong());
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		// [<step-2]
		System.out.println("\n--- Adding new metadata ---");

		// Define metadata key and value [>step-3]
		final String keyString = String.format(
			"username_%d", System.currentTimeMillis());
		final long scopedMetadataKey = Metadata.generateKey(keyString);
		final byte[] metadataValue =
			"alice".getBytes(StandardCharsets.UTF_8);
		// [<step-3]
		// Create the embedded metadata transaction [>step-4]
		final EmbeddedTransaction creationEmbeddedTx =
			facade.transactionFactory.createEmbedded(Map.of(
				"type", "account_metadata_transaction_v1",
				"signerPublicKey", signerKeyPair.getPublicKey(),
				"targetAddress", signerAddress,
				"scopedMetadataKey", scopedMetadataKey,
				// When creating new metadata, valueSizeDelta
				// equals the value length
				"valueSizeDelta", metadataValue.length,
				// Cast one value to infer Map<String, Object>,
				// as expected by the SDK.
				"value", (Object) metadataValue));
		System.out.println("Created embedded metadata transaction:");
		System.out.println(JSON_MAPPER
			.writerWithDefaultPrettyPrinter()
			.writeValueAsString(creationEmbeddedTx.toJson()));
		// [<step-4]
		// Build the aggregate transaction [>step-5]
		final List<EmbeddedTransaction> creationEmbeddedTxs =
			List.of(creationEmbeddedTx);
		final Transaction creationTx =
			facade.transactionFactory.create(Map.of(
				"type", "aggregate_complete_transaction_v3",
				"signerPublicKey", signerKeyPair.getPublicKey(),
				"deadline", timestamp.addHours(2).timestamp,
				"transactionsHash",
					SymbolFacade.hashEmbeddedTransactions(
						creationEmbeddedTxs),
				// Cast one value to infer Map<String, Object>,
				// as expected by the SDK.
				"transactions", (Object) creationEmbeddedTxs));
		creationTx.setFee(new Amount(
			FeeCalculator.calculateTransactionFee(
				creationTx, feeMultiplier)));
		// [<step-5]
		// Sign and generate final payload [>step-6]
		final String creationPayload = SymbolTransactionFactory
			.attachSignature(creationTx,
				facade.signTransaction(signerKeyPair, creationTx));

		// Announce and wait for confirmation
		final String creationTxHash =
			facade.hashTransaction(creationTx).toString();
		System.out.printf(
			"Built aggregate transaction with hash: %s%n",
			creationTxHash);
		announceTransaction(creationPayload, "creation transaction");
		waitForConfirmation(creationTxHash, "creation transaction");
		// [<step-6]
		System.out.println("\n--- Modifying existing metadata ---");

		// Fetch current metadata value from network [>step-7]
		final String scopedKeyHex = "%016X".formatted(
			scopedMetadataKey);
		final String metadataPath = String.format(
			"/metadata?sourceAddress=%s&targetAddress=%s"
				+ "&scopedMetadataKey=%s&metadataType=0",
			signerAddress, signerAddress, scopedKeyHex);
		System.out.printf("Fetching current metadata from %s%n",
			metadataPath);
		final HttpRequest metadataRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + metadataPath)).GET().build();
		final HttpResponse<String> metadataResponse = HTTP_CLIENT.send(
			metadataRequest, BodyHandlers.ofString());
		final JsonNode metadataJson = JSON_MAPPER.readTree(
			metadataResponse.body());

		// Get the metadata entry
		final JsonNode metadataData = metadataJson.get("data");
		if (null == metadataData)
			throw new IOException(
				"Unexpected metadata response: " + metadataJson);

		if (metadataData.isEmpty())
			throw new IOException("Metadata entry not found");

		final JsonNode metadataEntry = metadataData.get(0)
			.get("metadataEntry");
		final byte[] currentValue = java.util.HexFormat.of()
			.parseHex(metadataEntry.get("value").asText());
		System.out.printf("  Current value: %s%n",
			new String(currentValue, StandardCharsets.UTF_8));
		// [<step-7]
		// XOR the current and new values [>step-8]
		final byte[] newValue = "bob".getBytes(StandardCharsets.UTF_8);
		final byte[] updateValue = Metadata.updateValue(
			currentValue, newValue);

		// Create the update transaction with XOR'd value
		final EmbeddedTransaction updateEmbeddedTx =
			facade.transactionFactory.createEmbedded(Map.of(
				"type", "account_metadata_transaction_v1",
				"signerPublicKey", signerKeyPair.getPublicKey(),
				"targetAddress", signerAddress,
				"scopedMetadataKey", scopedMetadataKey,
				// valueSizeDelta is the difference in length
				// (can be negative)
				"valueSizeDelta",
					newValue.length - currentValue.length,
				// Cast one value to infer Map<String, Object>,
				// as expected by the SDK.
				"value", (Object) updateValue));
		// [<step-8]
		// Build the aggregate for the update [>step-9]
		final List<EmbeddedTransaction> updateEmbeddedTxs =
			List.of(updateEmbeddedTx);
		final Transaction updateTx =
			facade.transactionFactory.create(Map.of(
				"type", "aggregate_complete_transaction_v3",
				"signerPublicKey", signerKeyPair.getPublicKey(),
				"deadline", timestamp.addHours(2).timestamp,
				"transactionsHash",
					SymbolFacade.hashEmbeddedTransactions(
						updateEmbeddedTxs),
				// Cast one value to infer Map<String, Object>,
				// as expected by the SDK.
				"transactions", (Object) updateEmbeddedTxs));
		updateTx.setFee(new Amount(
			FeeCalculator.calculateTransactionFee(
				updateTx, feeMultiplier)));

		// Sign and announce the update
		final String updatePayload = SymbolTransactionFactory
			.attachSignature(updateTx,
				facade.signTransaction(signerKeyPair, updateTx));

		// Announce and wait for confirmation
		final String updateTxHash =
			facade.hashTransaction(updateTx).toString();
		System.out.printf(
			"Built aggregate transaction with hash: %s%n",
			updateTxHash);
		announceTransaction(updatePayload, "update transaction");
		waitForConfirmation(updateTxHash, "update transaction");
		// [<step-9]
	}
}
