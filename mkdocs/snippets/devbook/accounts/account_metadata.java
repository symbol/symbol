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

import com.fasterxml.jackson.core.JsonProcessingException;
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
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

final class AccountMetadata {
	private static final ObjectMapper OBJECT_MAPPER = new ObjectMapper();

	private static final String NODE_URL = System.getenv()
		.getOrDefault("NODE_URL", "https://reference.symboltest.net:3001");

	private static final SymbolFacade FACADE = new SymbolFacade("testnet");

	private AccountMetadata() {
	}

	private static JsonNode fetchJson(final String path)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + path)).GET().build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		return OBJECT_MAPPER.readTree(response.body());
	}

	private static void announceTransaction(
		final String payload,
		final String label)
		throws IOException, InterruptedException {
		System.out.println("Announcing " + label + " to /transactions");
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + "/transactions"))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		System.out.println("  Response: " + response.body());
	}

	private static void waitForConfirmation(
		final String transactionHash,
		final String label)
		throws IOException, InterruptedException {
		System.out.println("Waiting for " + label + " confirmation...");
		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			try {
				final JsonNode status = fetchJson(
					"/transactionStatus/" + transactionHash);
				final String group = status.get("group").asText();
				System.out.println("  Transaction status: " + group);
				if ("confirmed".equals(group)) {
					System.out.println(
						label + " confirmed in " + attempt + " seconds");
					return;
				}
				if ("failed".equals(group))
					throw new IOException(label + " failed: "
						+ status.get("code").asText());
			} catch (final JsonProcessingException ex) {
				System.out.println("  Transaction status: unknown");
			}
		}
		throw new IOException(label + " not confirmed after 60 seconds");
	}

	private static
	AggregateCompleteTransactionV3 createAggregateTransaction(
		final EmbeddedTransaction embeddedTransaction,
		final KeyPair signerKeyPair,
		final NetworkTimestamp timestamp,
		final long feeMultiplier) {
		final List<EmbeddedTransaction> embeddedTransactions = List.of(
			embeddedTransaction);
		final var descriptor = new
			AggregateCompleteTransactionV3Descriptor(
			SymbolFacade.hashEmbeddedTransactions(embeddedTransactions))
			.transactions(embeddedTransactions);
		final Map<String, Object> aggregateMap = descriptor.toMap();
		aggregateMap.put("signerPublicKey", signerKeyPair.getPublicKey());
		aggregateMap.put(
			"deadline", timestamp.addSeconds(7200).timestamp);
		final AggregateCompleteTransactionV3 transaction =
			(AggregateCompleteTransactionV3) FACADE.transactionFactory
				.create(aggregateMap);
		transaction.setFee(new Amount(
			FeeCalculator.calculateTransactionFee(
				transaction, feeMultiplier)));
		return transaction;
	}

	private static String signTransaction(
		final KeyPair signerKeyPair,
		final AggregateCompleteTransactionV3 transaction) {
		return SymbolTransactionFactory.attachSignature(
			transaction,
			FACADE.signTransaction(signerKeyPair, transaction));
	}

	private static String toMetadataKeyHex(final long value) {
		return "%016X".formatted(value);
	}

	private static String toPrettyJson(final Object value) {
		try {
			return OBJECT_MAPPER.writerWithDefaultPrettyPrinter()
				.writeValueAsString(value);
		} catch (final JsonProcessingException ex) {
			return value.toString();
		}
	}

	public static void main(final String[] args) {
		System.out.println("Using node " + NODE_URL);

		// [>step-1]
		final String privateKeyString = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY",
			"000000000000000000000000000000000000"
				+ "0000000000000000000000000000");
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(privateKeyString));

		final Address signerAddress = FACADE.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.println("Signer address: " + signerAddress);
		// [<step-1]
		try {
			// Fetch current network time [>step-2]
			final String timePath = "/node/time";
			System.out.println("Fetching current network time from "
				+ timePath);
			final JsonNode timeJson = fetchJson(timePath);
			final NetworkTimestamp timestamp = new NetworkTimestamp(
				timeJson.get("communicationTimestamps")
					.get("receiveTimestamp").asLong());
			System.out.println("  Network time: " + timestamp.timestamp
				+ " ms since nemesis");

			// Fetch recommended fees
			final String feePath = "/network/fees/transaction";
			System.out.println("Fetching recommended fees from "
				+ feePath);
			final JsonNode feeJson = fetchJson(feePath);
			final long feeMultiplier = Math.max(
				feeJson.get("medianFeeMultiplier").asLong(),
				feeJson.get("minFeeMultiplier").asLong());
			System.out.println("  Fee multiplier: " + feeMultiplier);
			// [<step-2]
			System.out.println("\n--- Adding new metadata ---");

			// Define metadata key and value [>step-3]
			final String keyString = "username_"
				+ System.currentTimeMillis();
			final long scopedMetadataKey = Metadata.generateKey(keyString);
			final byte[] metadataValue = "alice".getBytes(
				StandardCharsets.UTF_8);
			// [<step-3]
			// Create the embedded metadata transaction [>step-4]
			final var metadataDescriptor =
				new AccountMetadataTransactionV1Descriptor(
					signerAddress,
					scopedMetadataKey,
					// When creating new metadata, valueSizeDelta
					// equals the value length
					metadataValue.length)
					.value(metadataValue);
			final EmbeddedTransaction embeddedTransaction =
				FACADE.createEmbeddedTransactionFromTypedDescriptor(
					metadataDescriptor,
					signerKeyPair.getPublicKey());
			System.out.println("Created embedded metadata transaction:");
			System.out.println(toPrettyJson(
				embeddedTransaction.toJson()));
			// [<step-4]
			// Build the aggregate transaction [>step-5]
			final AggregateCompleteTransactionV3 transaction =
				createAggregateTransaction(
					embeddedTransaction,
					signerKeyPair,
					timestamp,
					feeMultiplier);
			// [<step-5]
			// Sign and generate final payload [>step-6]
			final String jsonPayload = signTransaction(
				signerKeyPair, transaction);

			// Announce and wait for confirmation
			final String transactionHash =
				FACADE.hashTransaction(transaction).toString();
			System.out.println(
				"Built aggregate transaction with hash: "
					+ transactionHash);
			announceTransaction(jsonPayload, "aggregate transaction");
			waitForConfirmation(transactionHash, "aggregate transaction");
			// [<step-6]
			System.out.println("\n--- Modifying existing metadata ---");

			// Fetch current metadata value from network [>step-7]
			final String scopedKeyHex = toMetadataKeyHex(
				scopedMetadataKey);
			final String metadataPath = "/metadata?sourceAddress="
				+ signerAddress
				+ "&targetAddress=" + signerAddress
				+ "&scopedMetadataKey=" + scopedKeyHex
				+ "&metadataType=0";
			System.out.println("Fetching current metadata from "
				+ metadataPath);
			final JsonNode metadataJson = fetchJson(metadataPath);

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
			System.out.println("  Current value: " + new String(
				currentValue, StandardCharsets.UTF_8));
			// [<step-7]
			// XOR the current and new values [>step-8]
			final byte[] newValue = "bob".getBytes(
				StandardCharsets.UTF_8);
			final byte[] updateValue = Metadata.updateValue(
				currentValue, newValue);

			// Create the update transaction with XOR'd value
			final var updateDescriptor =
				new AccountMetadataTransactionV1Descriptor(
					signerAddress,
					scopedMetadataKey,
					// valueSizeDelta is the difference in length
					// (can be negative)
					newValue.length - currentValue.length)
					.value(updateValue);
			final EmbeddedTransaction embeddedUpdate =
				FACADE.createEmbeddedTransactionFromTypedDescriptor(
					updateDescriptor,
					signerKeyPair.getPublicKey());
			// [<step-8]
			// Build the aggregate for the update [>step-9]
			final AggregateCompleteTransactionV3 updateTransaction =
				createAggregateTransaction(
					embeddedUpdate,
					signerKeyPair,
					timestamp,
					feeMultiplier);

			// Sign and announce the update
			final String updatePayload = signTransaction(
				signerKeyPair, updateTransaction);

			// Announce and wait for confirmation
			final String updateHash =
				FACADE.hashTransaction(updateTransaction).toString();
			System.out.println(
				"Built aggregate transaction with hash: " + updateHash);
			announceTransaction(updatePayload, "aggregate transaction");
			waitForConfirmation(updateHash, "aggregate transaction");
			// [<step-9]
		} catch (final IOException | InterruptedException ex) {
			System.out.println(ex.getMessage());
		}
	}
}
