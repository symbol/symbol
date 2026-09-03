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

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.Metadata;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class AccountMetadata {
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
			new AccountMetadata().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
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
		// Fetch recommended fees [>step-2]
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
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new AccountMetadataTransactionV1Descriptor(
					signerAddress,
					scopedMetadataKey,
					metadataValue.length,
					metadataValue),
				signerKeyPair.getPublicKey());
		System.out.println("Created embedded metadata transaction:");
		System.out.println(JSON_MAPPER
			.writerWithDefaultPrettyPrinter()
			.writeValueAsString(creationEmbeddedTx.toJson()));
		// [<step-4]
		// Build the aggregate transaction [>step-5]
		final List<EmbeddedTransaction> creationEmbeddedTxs =
			List.of(creationEmbeddedTx);
		final Transaction creationTx =
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						creationEmbeddedTxs),
					creationEmbeddedTxs,
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
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
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new AccountMetadataTransactionV1Descriptor(
					signerAddress,
					scopedMetadataKey,
					newValue.length - currentValue.length,
					updateValue),
				signerKeyPair.getPublicKey());
		// [<step-8]
		// Build the aggregate for the update [>step-9]
		final List<EmbeddedTransaction> updateEmbeddedTxs =
			List.of(updateEmbeddedTx);
		final Transaction updateTx =
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						updateEmbeddedTxs),
					updateEmbeddedTxs,
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);

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
