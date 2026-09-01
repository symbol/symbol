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

public final class AccountRestrictions {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private KeyPair signerKeyPair;

	private Address signerAddress;

	private final Address authAddress = new Address(
		"TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA");

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

	// Returns the restrictions currently applied to the account
	private JsonNode getAccountRestrictions( // [>step-3]
		final Address address
	) throws IOException, InterruptedException {
		final String restrictionsPath = String.format(
			"/restrictions/account/%s", address);
		System.out.printf("Getting restrictions from %s%n",
			restrictionsPath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + restrictionsPath)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT
			.send(request, BodyHandlers.ofString());
		if (2 != response.statusCode() / 100) {
			System.out.println("  Response: No restrictions found");
			return JSON_MAPPER.createArrayNode();
		}

		final JsonNode restrictions = JSON_MAPPER
			.readTree(response.body()).get("accountRestrictions")
			.get("restrictions");
		System.out.printf("  Response: %s%n", restrictions);
		return restrictions;
	} // [<step-3]

	// Returns a transaction that restricts an account
	private Transaction restrictionEnableTransaction( // [>step-5]
		final long feeMultiplier
	) throws IOException {
		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new AccountAddressRestrictionTransactionV1Descriptor(
					new AccountRestrictionFlags(
						AccountRestrictionFlags.ADDRESS.value |
						AccountRestrictionFlags.OUTGOING.value),
					List.of(authAddress),
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		System.out.println("Enabling the restriction with transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		return transaction;
	} // [<step-5]

	// Returns a transaction that removes a restriction from an account
	private Transaction restrictionDisableTransaction( // [>step-6]
		final long feeMultiplier,
		final JsonNode restriction
	) throws IOException {
		final List<Address> restrictionDeletions = new ArrayList<>();
		for (final JsonNode value : restriction.get("values"))
			restrictionDeletions.add(
				Address.fromDecodedAddressHexString(value.asText()));

		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new AccountAddressRestrictionTransactionV1Descriptor(
					new AccountRestrictionFlags(
						AccountRestrictionFlags.ADDRESS.value |
						AccountRestrictionFlags.OUTGOING.value),
					null,
					restrictionDeletions),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		System.out.println("Disabling the restriction with transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		return transaction;
	} // [<step-6]

	public static void main(final String[] args) {
		try {
			new AccountRestrictions().run();
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
		signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(privateKeyString));
		signerAddress = facade.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.printf("Signer address: %s%n", signerAddress);
		System.out.printf("Authorized address: %s%n", authAddress);
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
		// Get current state of the restriction and decide which
		// operation to perform
		// [>step-4]
		final JsonNode restrictions = getAccountRestrictions(
			signerAddress);
		Transaction transaction;
		if (restrictions.isEmpty()) {
			System.out.println("\n--- Enabling restriction ---");
			transaction = restrictionEnableTransaction(
				feeMultiplier);
		} else {
			System.out.println("\n--- Disabling restriction ---");
			transaction = restrictionDisableTransaction(
				feeMultiplier, restrictions.get(0));
		}
		// [<step-4]
		// Sign, announce and wait for confirmation [>step-7]
		String payload = SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(signerKeyPair, transaction));
		String hash = facade.hashTransaction(transaction).toString();
		announceTransaction(payload, "restriction transaction");
		waitForConfirmation(hash, "restriction transaction");
		// [<step-7]
		// [>step-8]
		// Try a dummy transfer to a random address with no mosaics
		transaction = facade.createTransactionFromTypedDescriptor(
			new TransferTransactionV1Descriptor(
				new Address("TBBHGE77IHHOIYA46B3XSORRNR2L5MLW54YO75Y"),
				null,
				null),
			signerKeyPair.getPublicKey(),
			feeMultiplier,
			2 * 60 * 60);
		payload = SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(signerKeyPair, transaction));
		hash = facade.hashTransaction(transaction).toString();
		System.out.println(
			"\n--- Attempting transfer to unauthorized address ---");
		announceTransaction(payload, "test transfer");
		waitForConfirmation(hash, "test transfer");
		// [<step-8]
	}
}
