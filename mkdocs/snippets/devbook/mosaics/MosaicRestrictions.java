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
import org.symbol.sdk.symbol.Restriction;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class MosaicRestrictions {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private KeyPair ownerKeyPair;

	private Address ownerAddress;

	private Address targetAddress;

	private long mosaicId;

	private long restrictionKey;

	// Helper function to announce a transaction
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

	// Helper function to wait for transaction confirmation
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

	// Returns restrictions currently applied to the mosaic
	// matching the given restriction key
	private List<JsonNode> getMosaicRestrictions( // [>step-4]
		final String query,
		final long key
	) throws IOException, InterruptedException {
		final String restrictionsPath = "/restrictions/mosaic?" + query;
		System.out.printf("  Getting restrictions from %s%n",
			restrictionsPath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + restrictionsPath)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		final List<JsonNode> result = new ArrayList<>();
		final JsonNode data = JSON_MAPPER.readTree(response.body())
			.get("data");
		if (!data.isEmpty()) {
			// Look at the first returned restriction
			final JsonNode restrictions = data.get(0)
				.get("mosaicRestrictionEntry").get("restrictions");
			// Filter by key
			for (final JsonNode restriction : restrictions) {
				final long restrictionValue = Long.parseUnsignedLong(
					restriction.get("key").asText());
				if (restrictionValue == key)
					result.add(restriction);
			}
		}
		System.out.printf("  Response: %s%n", result);
		return result;
	}

	private List<JsonNode> getMosaicGlobalRestrictions(
		final long queriedMosaicId,
		final long key
	) throws IOException, InterruptedException {
		return getMosaicRestrictions(String.format(
			"mosaicId=%016X&entryType=1", queriedMosaicId), key);
	} // [<step-4]

	private List<JsonNode> getMosaicAddressRestrictions( // [>step-5]
		final long queriedMosaicId,
		final Address address,
		final long key
	) throws IOException, InterruptedException {
		return getMosaicRestrictions(String.format(
			"mosaicId=%016X&entryType=0&targetAddress=%s",
			queriedMosaicId, address), key);
	} // [<step-5]

	// Returns a transaction enabling a mosaic's global restriction
	private EmbeddedTransaction setGlobalRestrictionTransaction()
		throws IOException {
		final EmbeddedTransaction transaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MosaicGlobalRestrictionTransactionV1Descriptor(
					new UnresolvedMosaicId(mosaicId),
					new UnresolvedMosaicId(0),
					restrictionKey,
					0,
					1,
					MosaicRestrictionType.NONE,
					MosaicRestrictionType.GE),
				ownerKeyPair.getPublicKey());
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		return transaction;
	}

	// Returns a transaction setting an address restriction's value
	private EmbeddedTransaction addressRestrictionSetValue(
		final long previousValue,
		final long newValue,
		final Address address
	) throws IOException {
		final EmbeddedTransaction transaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new MosaicAddressRestrictionTransactionV1Descriptor(
					new UnresolvedMosaicId(mosaicId),
					restrictionKey,
					previousValue,
					newValue,
					address),
				ownerKeyPair.getPublicKey());
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		return transaction;
	}

	public static void main(final String[] args) {
		try {
			new MosaicRestrictions().run();
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
			"OWNER_PRIVATE_KEY", "0".repeat(64));
		ownerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(privateKeyString));
		ownerAddress = facade.network.publicKeyToAddress(
			ownerKeyPair.getPublicKey());
		System.out.printf("Owner address: %s%n", ownerAddress);

		targetAddress = new Address(System.getenv().getOrDefault(
			"TARGET_ADDRESS", "TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA"));
		System.out.printf("Target address: %s%n", targetAddress);

		mosaicId = Long.parseUnsignedLong(System.getenv().getOrDefault(
			"MOSAIC_ID", "6A5ACF2376E50D4A"), 16);
		System.out.printf("Mosaic ID: 0x%016X%n", mosaicId);

		final String restrictionName = System.getenv().getOrDefault(
			"RESTRICTION_NAME", "security_level");
		restrictionKey = Restriction.mosaicRestrictionGenerateKey(
			restrictionName);
		System.out.printf("Restriction name: \"%s\" (key: 0x%016X)%n",
			restrictionName, restrictionKey); // [<step-1]

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
		// Enable global restriction if required [>step-3]
		final List<EmbeddedTransaction> transactions = new ArrayList<>();
		System.out.println(
			"Checking if the global restriction is enabled:");
		final List<JsonNode> globalRestrictions =
			getMosaicGlobalRestrictions(mosaicId, restrictionKey);
		if (globalRestrictions.isEmpty()) {
			// Enable the global restriction
			System.out.println("+ Enabling global restriction");
			transactions.add(setGlobalRestrictionTransaction());

			// Enable the address restriction
			System.out.println("+ Authorizing owner account");
			transactions.add(addressRestrictionSetValue(
				0xFFFFFFFFFFFFFFFFL, 1, ownerAddress));
		}
		// [<step-3]
		// Toggle target address restriction
		System.out.println( // [>step-6]
			"Checking if target account is authorized:");
		final List<JsonNode> addressRestrictions =
			getMosaicAddressRestrictions(
				mosaicId, targetAddress, restrictionKey);
		long previousValue = 0xFFFFFFFFFFFFFFFFL;
		if (!addressRestrictions.isEmpty())
			previousValue = Long.parseUnsignedLong(
				addressRestrictions.get(0).get("value").asText());
		if (1 != previousValue) {
			// Enable the address restriction
			System.out.println("+ Authorizing target account");
			transactions.add(addressRestrictionSetValue(
				previousValue, 1, targetAddress));
		} else {
			// Disable the address restriction
			System.out.println("+ Deauthorizing target account");
			transactions.add(addressRestrictionSetValue(
				previousValue, 0, targetAddress));
		}
		// [<step-6]
		// Build an aggregate transaction
		System.out.printf( // [>step-7]
			"Bundling %d transaction(s) in an aggregate%n",
			transactions.size());
		Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new AggregateCompleteTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(transactions),
					transactions,
					null),
				ownerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-7]
		// Sign, announce and wait for confirmation
		// [>step-8]
		String payload = SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(ownerKeyPair, transaction));
		String transactionHash =
			facade.hashTransaction(transaction).toString();
		announceTransaction(payload, "aggregate");
		waitForConfirmation(transactionHash, "aggregate");
		// [<step-8]
		// Try to transfer the mosaic to the target address
		// [>step-9]
		transaction = facade.createTransactionFromTypedDescriptor(
			new TransferTransactionV1Descriptor(
				targetAddress,
				List.of(new UnresolvedMosaicDescriptor(
					new UnresolvedMosaicId(mosaicId),
					new Amount(1))),
				null),
			ownerKeyPair.getPublicKey(),
			feeMultiplier,
			2 * 60 * 60);
		payload = SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(ownerKeyPair, transaction));
		transactionHash = facade.hashTransaction(transaction).toString();
		System.out.println("\nAttempting transfer to the target account");
		announceTransaction(payload, "test transfer");
		waitForConfirmation(transactionHash, "test transfer");
		// [<step-9]
	}
}
