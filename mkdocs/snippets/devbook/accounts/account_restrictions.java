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

final class AccountRestrictions {
	private static final ObjectMapper OBJECT_MAPPER = new ObjectMapper();

	private static final String NODE_URL = System.getenv()
		.getOrDefault("NODE_URL", "https://reference.symboltest.net:3001");

	private static final SymbolFacade FACADE = new SymbolFacade("testnet");

	private static final KeyPair SIGNER_KEY_PAIR;

	private static final Address SIGNER_ADDRESS;

	private static final Address AUTHORIZED_ADDRESS = new Address(
		"TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA");

	static {
		// [>step-1]
		final String privateKeyString = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY",
			"000000000000000000000000000000000000"
				+ "0000000000000000000000000000");
		SIGNER_KEY_PAIR = new KeyPair(
			new CryptoTypes.PrivateKey(privateKeyString));
		SIGNER_ADDRESS = FACADE.network.publicKeyToAddress(
			SIGNER_KEY_PAIR.getPublicKey());
		System.out.println("Signer address: " + SIGNER_ADDRESS);
		System.out.println("Authorized address: " + AUTHORIZED_ADDRESS);
		// [<step-1]
	}

	private AccountRestrictions() {
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

	// Returns the list of restrictions currently applied to the account
	private static List<JsonNode> getAccountRestrictions( // [>step-3]
		final Address address)
		throws IOException, InterruptedException {
		final String restrictionsPath = "/restrictions/account/" + address;
		System.out.println("Getting restrictions from "
			+ restrictionsPath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + restrictionsPath)).GET().build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		if (2 != response.statusCode() / 100) {
			System.out.println("  Response: No restrictions found");
			return List.of();
		}

		final JsonNode restrictions = OBJECT_MAPPER
			.readTree(response.body())
			.get("accountRestrictions")
			.get("restrictions");
		System.out.println("  Response: " + restrictions);

		final List<JsonNode> result = new ArrayList<>();
		restrictions.forEach(result::add);
		return result;
	} // [<step-3]

	// Returns a transaction that restricts an account
	private static Transaction restrictionEnableTransaction( // [>step-5]
		final NetworkTimestamp timestamp,
		final long feeMultiplier) {
		final var descriptor = new
			AccountAddressRestrictionTransactionV1Descriptor(
				new AccountRestrictionFlags(
					AccountRestrictionFlags.ADDRESS.value
						| AccountRestrictionFlags.OUTGOING.value))
				.restrictionAdditions(List.of(AUTHORIZED_ADDRESS));
		final Transaction transaction = createTransaction(
			descriptor.toMap(), timestamp, feeMultiplier);
		System.out.println("Enabling the restriction with transaction:");
		System.out.println(toPrettyJson(transaction.toJson()));

		return transaction;
	} // [<step-5]

	// Returns a transaction that removes a restriction from an account
	private static Transaction restrictionDisableTransaction( // [>step-6]
		final NetworkTimestamp timestamp,
		final long feeMultiplier,
		final JsonNode restriction) {
		final List<Address> restrictionDeletions = new ArrayList<>();
		for (final JsonNode value : restriction.get("values"))
			restrictionDeletions.add(
				Address.fromDecodedAddressHexString(value.asText()));

		final var descriptor = new
			AccountAddressRestrictionTransactionV1Descriptor(
				new AccountRestrictionFlags(
					AccountRestrictionFlags.ADDRESS.value
						| AccountRestrictionFlags.OUTGOING.value))
				.restrictionDeletions(restrictionDeletions);
		final Transaction transaction = createTransaction(
			descriptor.toMap(), timestamp, feeMultiplier);
		System.out.println("Disabling the restriction with transaction:");
		System.out.println(toPrettyJson(transaction.toJson()));

		return transaction;
	} // [<step-6]

	private static Transaction createTransaction(
		final Map<String, Object> descriptor,
		final NetworkTimestamp timestamp,
		final long feeMultiplier) {
		descriptor.put("signerPublicKey", SIGNER_KEY_PAIR.getPublicKey());
		descriptor.put(
			"deadline", timestamp.addSeconds(7200).timestamp);
		final Transaction transaction =
			FACADE.transactionFactory.create(descriptor);
		transaction.setFee(new Amount(
			FeeCalculator.calculateTransactionFee(
				transaction, feeMultiplier)));
		return transaction;
	}

	private static String signTransaction(final Transaction transaction) {
		return SymbolTransactionFactory.attachSignature(
			transaction,
			FACADE.signTransaction(SIGNER_KEY_PAIR, transaction));
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
			// Get current state of the restriction and decide which
			// operation to perform
			final List<JsonNode> restrictions = getAccountRestrictions(
				SIGNER_ADDRESS); // [>step-4]
			Transaction transaction;
			if (restrictions.isEmpty()) {
				System.out.println("\n--- Enabling restriction ---");
				transaction = restrictionEnableTransaction(
					timestamp, feeMultiplier);
			} else {
				System.out.println("\n--- Disabling restriction ---");
				transaction = restrictionDisableTransaction(
					timestamp, feeMultiplier, restrictions.get(0));
			}
			// [<step-4]
			// Sign, announce and wait for confirmation [>step-7]
			String payload = signTransaction(transaction);
			String hash = FACADE.hashTransaction(transaction).toString();
			announceTransaction(payload, "restriction transaction");
			waitForConfirmation(hash, "restriction transaction");
			// [<step-7]
			// [>step-8]
			// Try a dummy transfer to a random address with no mosaics
			final var transferDescriptor =
				new TransferTransactionV1Descriptor(
					"TBBHGE77IHHOIYA46B3XSORRNR2L5MLW54YO75Y");
			transaction = createTransaction(
				transferDescriptor.toMap(), timestamp, feeMultiplier);
			payload = signTransaction(transaction);
			hash = FACADE.hashTransaction(transaction).toString();
			System.out.println(
				"\n--- Attempting transfer to unauthorized address ---");
			announceTransaction(payload, "test transfer");
			waitForConfirmation(hash, "test transfer");
			// [<step-8]
		} catch (final IOException | InterruptedException ex) {
			System.out.println(ex.getMessage());
		}
	}
}
