//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.FeeCalculator;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.NetworkTimestamp;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.models.Amount;
import org.symbol.sdk.symbol.models.Transaction;

public final class ManualTransactionCreation {
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
			new ManualTransactionCreation().run();
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
			new CryptoTypes.PrivateKey(signerPrivateKey)); // [<step-1]

		// Fetch current network time [>step-2]
		final String timePath = "/node/time";
		System.out.printf("Fetching current network time from %s%n",
			timePath);
		final HttpRequest timeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + timePath)).GET().build();
		final HttpResponse<String> timeResponse = HTTP_CLIENT.send(
			timeRequest, BodyHandlers.ofString());
		final JsonNode timeJSON = JSON_MAPPER.readTree(
			timeResponse.body());
		final NetworkTimestamp timestamp = new NetworkTimestamp(
			timeJSON.get("communicationTimestamps")
				.get("receiveTimestamp").asLong());
		System.out.printf("  Network time: %d ms since nemesis%n",
			timestamp.timestamp); // [<step-2]

		// Fetch recommended fees [>step-3]
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
		// [<step-3]
		// Build the transaction [>step-4]
		final Transaction transaction = facade.transactionFactory.create(
			Map.of(
				"type", "transfer_transaction_v1",
				"signerPublicKey", signerKeyPair.getPublicKey().toString(),
				"deadline", timestamp.addHours(2).timestamp,
				"recipientAddress", facade.network.publicKeyToAddress(
					signerKeyPair.getPublicKey()).toString(),
				"mosaics", List.of(Map.of(
					"mosaicId", IdGenerator.generateMosaicAliasId(
						"symbol.xym"),
					"amount", 1_000_000L // 1 XYM
				))
			));
		transaction.setFee(new Amount(
			FeeCalculator.calculateTransactionFee(
				transaction, feeMultiplier))); // [<step-4]

		// Sign transaction and generate final payload [>step-5]
		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, transaction);
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(transaction, signature);
		System.out.println("Built transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson())); // [<step-5]

		// Announce the transaction [>step-6]
		announceTransaction(jsonPayload, "transaction"); // [<step-6]

		// Wait for confirmation [>step-7]
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf("Transaction hash: %s%n", transactionHash);
		waitForConfirmation(transactionHash, "transaction"); // [<step-7]
	}
}
