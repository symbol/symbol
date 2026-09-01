//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.ArrayList;
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
import org.symbol.sdk.symbol.NetworkTimestamp;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

final class ConfigureMultisig {
	private static final ObjectMapper OBJECT_MAPPER = new ObjectMapper();

	private static final String NODE_URL = System.getenv()
		.getOrDefault("NODE_URL", "https://reference.symboltest.net:3001");

	private static final SymbolFacade FACADE = new SymbolFacade("testnet");

	private static final String KEY_PREFIX = "0".repeat(63);

	private static final KeyPair MULTISIG_KEY_PAIR;

	private static final Address MULTISIG_ADDRESS;

	private static final List<KeyPair> COSIGNATORY_KEY_PAIRS =
		new ArrayList<>();

	private static final List<Address> COSIGNATORY_ADDRESSES =
		new ArrayList<>();

	static {
		// [>step-1]
		final String multisigPrivateKey = System.getenv().getOrDefault(
			"MULTISIG_PRIVATE_KEY", KEY_PREFIX + "1");
		MULTISIG_KEY_PAIR = new KeyPair(
			new CryptoTypes.PrivateKey(multisigPrivateKey));
		MULTISIG_ADDRESS = FACADE.network.publicKeyToAddress(
			MULTISIG_KEY_PAIR.getPublicKey());
		System.out.println("Multisig address: " + MULTISIG_ADDRESS
			+ " (public key " + MULTISIG_KEY_PAIR.getPublicKey() + ")");

		for (int i = 0; 2 > i; ++i) {
			final String cosignatoryPrivateKey = System.getenv()
				.getOrDefault(
					"COSIGNATORY" + i + "_PRIVATE_KEY",
					KEY_PREFIX + (i + 2));
			final KeyPair keyPair = new KeyPair(
				new CryptoTypes.PrivateKey(cosignatoryPrivateKey));
			COSIGNATORY_KEY_PAIRS.add(keyPair);
			final Address address = FACADE.network.publicKeyToAddress(
				keyPair.getPublicKey());
			COSIGNATORY_ADDRESSES.add(address);
			System.out.println("Cosignatory " + i + " address: " + address
				+ " (public key " + keyPair.getPublicKey() + ")");
		}
		// [<step-1]
	}

	private ConfigureMultisig() {
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

	// [>step-3]
	// Returns the cosignatory addresses of the provided multisig account,
	// or an empty list if the account is not multisig or has
	// never been used
	private static List<String> getMultisigCosignatories(
		final Address address)
		throws IOException, InterruptedException {
		final String multisigPath = "/account/" + address + "/multisig";
		System.out.println("Getting cosignatories from " + multisigPath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + multisigPath)).GET().build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		if (2 != response.statusCode() / 100) {
			System.out.println("  Response: No cosignatories");
			return List.of();
		}

		final JsonNode cosignatories = OBJECT_MAPPER
			.readTree(response.body())
			.get("multisig")
			.get("cosignatoryAddresses");
		System.out.println("  Response: " + cosignatories);

		final List<String> result = new ArrayList<>();
		cosignatories.forEach(addressNode -> result.add(
			addressNode.asText()));
		return result;
	} // [<step-3]

	// Returns a transaction that turns a regular account into a multisig
	private static
	AggregateCompleteTransactionV3 multisigEnableTransaction(
		final NetworkTimestamp timestamp,
		final long feeMultiplier) {
		// [>step-5]
		// Create an embedded multisig account modification transaction
		// that adds two cosignatories
		final var embeddedDescriptor = new
			MultisigAccountModificationTransactionV1Descriptor(1, 1)
				.addressAdditions(COSIGNATORY_ADDRESSES);
		final EmbeddedTransaction embeddedTransaction =
			FACADE.createEmbeddedTransactionFromTypedDescriptor(
				embeddedDescriptor, MULTISIG_KEY_PAIR.getPublicKey());
		// [<step-5]
		// Build the aggregate transaction [>step-6]
		final List<EmbeddedTransaction> embeddedTransactions = List.of(
			embeddedTransaction);
		final AggregateCompleteTransactionV3 transaction =
			createAggregateTransaction(
				embeddedTransactions,
				MULTISIG_KEY_PAIR,
				timestamp,
				feeMultiplier,
				COSIGNATORY_KEY_PAIRS.size());
		System.out.println(
			"Enabling the multisig with the aggregate transaction:");
		System.out.println(toPrettyJson(transaction.toJson()));
		// [<step-6]
		// [>step-7]
		// Sign the aggregate transaction with the multisig's signature
		SymbolTransactionFactory.attachSignature(
			transaction,
			FACADE.signTransaction(MULTISIG_KEY_PAIR, transaction));

		// Append signatures from all cosignatories
		final List<Cosignature> cosignatures = new ArrayList<>();
		for (final KeyPair cosignatoryKeyPair : COSIGNATORY_KEY_PAIRS)
			cosignatures.add(FACADE.cosignTransaction(
				cosignatoryKeyPair, transaction));
		transaction.setCosignatures(cosignatures);
		// [<step-7]
		return transaction;
	}

	// Returns a transaction that turns a multisig into a regular account
	private static
	AggregateCompleteTransactionV3 multisigDisableTransaction(
		final NetworkTimestamp timestamp,
		final long feeMultiplier) {
		// [>step-8]
		// Create two embedded multisig account modification transactions
		// because cosignatories must be removed one by one
		final var embeddedDescriptor1 = new
			MultisigAccountModificationTransactionV1Descriptor(0, 0)
				.addressDeletions(List.of(COSIGNATORY_ADDRESSES.get(1)));
		final EmbeddedTransaction embeddedTransaction1 =
			FACADE.createEmbeddedTransactionFromTypedDescriptor(
				embeddedDescriptor1, MULTISIG_KEY_PAIR.getPublicKey());
		final var embeddedDescriptor2 = new
			MultisigAccountModificationTransactionV1Descriptor(-1, -1)
				.addressDeletions(List.of(COSIGNATORY_ADDRESSES.get(0)));
		final EmbeddedTransaction embeddedTransaction2 =
			FACADE.createEmbeddedTransactionFromTypedDescriptor(
				embeddedDescriptor2, MULTISIG_KEY_PAIR.getPublicKey());
		// [<step-8]
		// Build the aggregate transaction [>step-9]
		final List<EmbeddedTransaction> embeddedTransactions = List.of(
			embeddedTransaction1, embeddedTransaction2);
		final AggregateCompleteTransactionV3 transaction =
			createAggregateTransaction(
				embeddedTransactions,
				COSIGNATORY_KEY_PAIRS.get(0),
				timestamp,
				feeMultiplier,
				0);
		System.out.println(
			"Disabling the multisig with the aggregate transaction:");
		System.out.println(toPrettyJson(transaction.toJson()));

		SymbolTransactionFactory.attachSignature(
			transaction,
			FACADE.signTransaction(
				COSIGNATORY_KEY_PAIRS.get(0), transaction));
		// [<step-9]
		return transaction;
	}

	private static
	AggregateCompleteTransactionV3 createAggregateTransaction(
		final List<EmbeddedTransaction> embeddedTransactions,
		final KeyPair signerKeyPair,
		final NetworkTimestamp timestamp,
		final long feeMultiplier,
		final int cosignatureCount) {
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
				transaction, feeMultiplier, cosignatureCount)));
		return transaction;
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
			// [>step-4]
			// Get current state of the multisig account and decide which
			// operation to perform
			final List<String> cosignatories = getMultisigCosignatories(
				MULTISIG_ADDRESS);
			final AggregateCompleteTransactionV3 transaction;
			if (cosignatories.isEmpty())
				transaction = multisigEnableTransaction(
					timestamp, feeMultiplier);
			else
				transaction = multisigDisableTransaction(
					timestamp, feeMultiplier);

			final String payload =
				SymbolTransactionFactory.toJson(transaction);
			// [<step-4]
			// Announce and wait for confirmation [>step-10]
			final String transactionHash =
				FACADE.hashTransaction(transaction).toString();
			System.out.println(
				"Built aggregate transaction with hash: "
					+ transactionHash);
			announceTransaction(payload, "aggregate transaction");
			waitForConfirmation(transactionHash, "aggregate transaction");
			// [<step-10]
		} catch (final IOException | InterruptedException ex) {
			System.out.println(ex.getMessage());
		}
	}
}
