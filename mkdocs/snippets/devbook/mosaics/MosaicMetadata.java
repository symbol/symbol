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

public final class MosaicMetadata {
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
			new MosaicMetadata().run();
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

		// Get mosaic ID from environment
		final String mosaicIdHex = System.getenv().getOrDefault(
			"MOSAIC_ID", "6D1314BE751B62C2");
		final long mosaicId = Long.parseUnsignedLong(mosaicIdHex, 16);
		System.out.printf("Mosaic ID: %d (0x%016X)%n",
			mosaicId, mosaicId);
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
			"description_%d", System.currentTimeMillis() / 1000);
		final long scopedMetadataKey = Metadata.generateKey(keyString);
		final byte[] metadataValue =
			"My first mosaic".getBytes(StandardCharsets.UTF_8);
		// [<step-3]
		// Create the embedded metadata transaction [>step-4]
		final EmbeddedTransaction embeddedTransaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MosaicMetadataTransactionV1Descriptor(
					signerAddress,
					scopedMetadataKey,
					new UnresolvedMosaicId(mosaicId),
					// When creating new metadata, valueSizeDelta
					// equals value length
					metadataValue.length,
					metadataValue),
				signerKeyPair.getPublicKey());
		System.out.println("Created embedded metadata transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(embeddedTransaction.toJson()));
		// [<step-4]
		// Build the aggregate transaction [>step-5]
		final List<EmbeddedTransaction> embeddedTransactions =
			List.of(embeddedTransaction);
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
		// [<step-5]
		// Sign and generate final payload [>step-6]
		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, transaction);
		String jsonPayload = SymbolTransactionFactory.attachSignature(
			transaction, signature);

		// Announce and wait for confirmation
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf("Built aggregate transaction with hash: %s%n",
			transactionHash);
		announceTransaction(jsonPayload, "aggregate transaction");
		waitForConfirmation(transactionHash, "aggregate transaction");
		// [<step-6]
		System.out.println("\n--- Modifying existing metadata ---");

		// Fetch current metadata value from network [>step-7]
		final String metadataPath = String.format(
			"/metadata?sourceAddress=%s&targetAddress=%s"
				+ "&scopedMetadataKey=%016X&targetId=%016X"
				+ "&metadataType=1",
			signerAddress, signerAddress, scopedMetadataKey, mosaicId);
		System.out.printf("Fetching current metadata from %s%n",
			metadataPath);
		final HttpRequest metadataRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + metadataPath)).GET().build();
		final HttpResponse<String> metadataResponse = HTTP_CLIENT.send(
			metadataRequest, BodyHandlers.ofString());
		final JsonNode metadataJson = JSON_MAPPER.readTree(
			metadataResponse.body());

		// Get the metadata entry
		if (metadataJson.get("data").isEmpty())
			throw new IOException("Metadata entry not found");
		final JsonNode metadataEntry = metadataJson.get("data").get(0)
			.get("metadataEntry");
		final byte[] currentValue = java.util.HexFormat.of()
			.parseHex(metadataEntry.get("value").asText());
		System.out.printf("  Current value: %s%n",
			new String(currentValue, StandardCharsets.UTF_8));
		// [<step-7]
		// XOR the current and new values [>step-8]
		final byte[] newValue =
			"Updated mosaic".getBytes(StandardCharsets.UTF_8);
		final byte[] updateValue = Metadata.updateValue(
			currentValue, newValue);

		// Create the update transaction with XOR'd value
		final EmbeddedTransaction embeddedUpdate =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MosaicMetadataTransactionV1Descriptor(
					signerAddress,
					scopedMetadataKey,
					new UnresolvedMosaicId(mosaicId),
					// valueSizeDelta is the difference in length
					// (can be negative)
					newValue.length - currentValue.length,
					updateValue),
				signerKeyPair.getPublicKey());
		System.out.println("Created embedded update transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(embeddedUpdate.toJson()));
		// [<step-8]
		// Build the aggregate for the update [>step-9]
		final List<EmbeddedTransaction> updateEmbeddedTransactions =
			List.of(embeddedUpdate);
		final Transaction updateTransaction =
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						updateEmbeddedTransactions),
					updateEmbeddedTransactions,
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);

		// Sign the update
		final CryptoTypes.Signature updateSignature =
			facade.signTransaction(signerKeyPair, updateTransaction);
		final String updatePayload = SymbolTransactionFactory
			.attachSignature(updateTransaction, updateSignature);

		// Announce and wait for confirmation
		final String updateHash =
			facade.hashTransaction(updateTransaction).toString();
		System.out.printf("Built aggregate transaction with hash: %s%n",
			updateHash);
		announceTransaction(updatePayload, "aggregate transaction");
		waitForConfirmation(updateHash, "aggregate transaction");
		// [<step-9]
	}
}
