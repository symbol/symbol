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

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

final class ConfigureMultisig {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private static final String KEY_PREFIX = "0".repeat(63);

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private KeyPair multisigKeyPair;

	private Address multisigAddress;

	private final List<KeyPair> cosignatoryKeyPairs =
		new ArrayList<>();

	private final List<Address> cosignatoryAddresses =
		new ArrayList<>();

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

	// [>step-3]
	// Returns the cosignatory addresses of the provided multisig account,
	// or an empty array if the account is not multisig or has
	// never been used
	private JsonNode getMultisigCosignatories(
		final Address address
	) throws IOException, InterruptedException {
		final String multisigPath = String.format(
			"/account/%s/multisig", address);
		System.out.printf("Getting cosignatories from %s%n",
			multisigPath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + multisigPath)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT
			.send(request, BodyHandlers.ofString());
		if (2 != response.statusCode() / 100) {
			System.out.println("  Response: No cosignatories");
			return JSON_MAPPER.createArrayNode();
		}

		final JsonNode cosignatories = JSON_MAPPER
			.readTree(response.body()).get("multisig")
			.get("cosignatoryAddresses");
		System.out.printf("  Response: %s%n", cosignatories);
		return cosignatories;
	} // [<step-3]

	// Returns a transaction that turns a regular account into a multisig
	private Transaction multisigEnableTransaction(
		final long feeMultiplier
	) throws IOException {
		// [>step-5]
		// Create an embedded multisig account modification transaction
		// that adds two cosignatories
		final EmbeddedTransaction embeddedTransaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MultisigAccountModificationTransactionV1Descriptor(
					1,
					1,
					cosignatoryAddresses,
					null),
				multisigKeyPair.getPublicKey());
		// [<step-5]
		// Build the aggregate transaction [>step-6]
		final List<EmbeddedTransaction> embeddedTransactions = List.of(
			embeddedTransaction);
		final var transaction = (AggregateCompleteTransactionV3)
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						embeddedTransactions),
					embeddedTransactions,
					null),
				multisigKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60,
				cosignatoryKeyPairs.size());
		System.out.println(
			"Enabling the multisig with the aggregate transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));
		// [<step-6]
		// [>step-7]
		// Sign the aggregate transaction with the multisig's signature
		SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(multisigKeyPair, transaction));

		// Append signatures from all cosignatories
		final List<Cosignature> cosignatures = new ArrayList<>();
		for (final KeyPair cosignatoryKeyPair : cosignatoryKeyPairs)
			cosignatures.add(facade.cosignTransaction(
				cosignatoryKeyPair, transaction));
		transaction.setCosignatures(cosignatures);
		// [<step-7]
		return transaction;
	}

	// Returns a transaction that turns a multisig into a regular account
	private Transaction multisigDisableTransaction(
		final long feeMultiplier
	) throws IOException {
		// [>step-8]
		// Create two embedded multisig account modification transactions
		// because cosignatories must be removed one by one
		final EmbeddedTransaction embeddedTransaction1 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MultisigAccountModificationTransactionV1Descriptor(
					0,
					0,
					null,
					List.of(cosignatoryAddresses.get(1))),
				multisigKeyPair.getPublicKey());
		final EmbeddedTransaction embeddedTransaction2 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MultisigAccountModificationTransactionV1Descriptor(
					-1,
					-1,
					null,
					List.of(cosignatoryAddresses.get(0))),
				multisigKeyPair.getPublicKey());
		// [<step-8]
		// Build the aggregate transaction [>step-9]
		final List<EmbeddedTransaction> embeddedTransactions = List.of(
			embeddedTransaction1, embeddedTransaction2);
		final var transaction = (AggregateCompleteTransactionV3)
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						embeddedTransactions),
					embeddedTransactions,
					null),
				cosignatoryKeyPairs.get(0).getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		System.out.println(
			"Disabling the multisig with the aggregate transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(
				cosignatoryKeyPairs.get(0), transaction));
		// [<step-9]
		return transaction;
	}

	public static void main(final String[] args) {
		try {
			new ConfigureMultisig().run();
		} catch (final Exception ex) {
			System.out.printf("%s%n", ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		final String multisigPrivateKey = System.getenv().getOrDefault(
			"MULTISIG_PRIVATE_KEY", KEY_PREFIX + "1");
		multisigKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(multisigPrivateKey));
		multisigAddress = facade.network.publicKeyToAddress(
			multisigKeyPair.getPublicKey());
		System.out.printf("Multisig address: %s (public key %s)%n",
			multisigAddress, multisigKeyPair.getPublicKey());

		for (int i = 0; 2 > i; ++i) {
			final String cosignatoryPrivateKey =
				System.getenv().getOrDefault(
					String.format("COSIGNATORY%d_PRIVATE_KEY", i),
					KEY_PREFIX + (i + 2));
			final KeyPair keyPair = new KeyPair(
				new CryptoTypes.PrivateKey(cosignatoryPrivateKey));
			cosignatoryKeyPairs.add(keyPair);
			final Address address = facade.network.publicKeyToAddress(
				keyPair.getPublicKey());
			cosignatoryAddresses.add(address);
			System.out.printf(
				"Cosignatory %d address: %s (public key %s)%n",
				i, address, keyPair.getPublicKey());
		}
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
		// [>step-4]
		// Get current state of the multisig account and decide which
		// operation to perform
		final JsonNode cosignatories =
			getMultisigCosignatories(multisigAddress);
		final Transaction transaction;
		if (cosignatories.isEmpty())
			transaction = multisigEnableTransaction(
				feeMultiplier);
		else
			transaction = multisigDisableTransaction(
				feeMultiplier);

		final String payload =
			SymbolTransactionFactory.toJson(transaction);
		// [<step-4]
		// Announce and wait for confirmation [>step-10]
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf(
			"Built aggregate transaction with hash: %s%n",
			transactionHash);
		announceTransaction(payload, "aggregate transaction");
		waitForConfirmation(transactionHash, "aggregate transaction");
		// [<step-10]
	}
}
