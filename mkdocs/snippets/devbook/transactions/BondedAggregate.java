//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class BondedAggregate {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private KeyPair accountAKeyPair;

	private KeyPair accountBKeyPair;

	private Address accountAAddress;

	private Address accountBAddress;

	// Helper function to announce transaction
	private void announceTransaction(
		final String payload,
		final String endpoint,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Announcing %s to %s%n", label, endpoint);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + endpoint))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", response.body());
	}

	// Helper function to wait for transaction status
	private void waitForStatus(
		final String hash,
		final String expectedStatus,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf(
			"Waiting for %s to reach %s status...%n",
			label, expectedStatus);
		int attempts = 0;
		final int maxAttempts = 60;

		while (attempts < maxAttempts) {
			final String url = nodeUrl + "/transactionStatus/" + hash;
			final HttpRequest request = HttpRequest.newBuilder(
				URI.create(url)).GET().build();
			final HttpResponse<String> response = HTTP_CLIENT.send(
				request, BodyHandlers.ofString());

			if (response.statusCode() / 100 != 2) {
				if (404 == response.statusCode()) {
					System.out.println(
						"  Transaction status: not yet available");
				} else
					throw new IOException(
						"HTTP " + response.statusCode());
			} else {
				final JsonNode status = JSON_MAPPER.readTree(
					response.body());

				System.out.printf("  Transaction status: %s%n",
					status.get("group").asText());

				if ("failed".equals(status.get("group").asText()))
					throw new IOException(String.format("%s failed: %s",
						label, status.get("code").asText()));

				if (status.get("group").asText().equals(expectedStatus)) {
					System.out.printf("%s %s in %d seconds%n",
						label, expectedStatus, attempts);
					return;
				}
			}

			++attempts;
			Thread.sleep(1000);
		}

		throw new IOException(String.format(
			"%s not %s after %d attempts",
			label, expectedStatus, maxAttempts));
	}

	public static void main(final String[] args) {
		try {
			new BondedAggregate().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		// Account A (initiates the aggregate tx
		// and sends XYM to Account B)
		final String accountAPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_A_PRIVATE_KEY", "0".repeat(64));
		accountAKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountAPrivateKey));

		// Account B (sends custom mosaic to Account A)
		final String accountBPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_B_PRIVATE_KEY", "1".repeat(64));
		accountBKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountBPrivateKey));

		accountAAddress = facade.network.publicKeyToAddress(
			accountAKeyPair.getPublicKey());
		accountBAddress = facade.network.publicKeyToAddress(
			accountBKeyPair.getPublicKey());
		System.out.printf("Account A: %s%n", accountAAddress);
		System.out.printf("Account B: %s%n", accountBAddress);
		// [<step-1]

		// Fetch recommended fees [>step-2]
		final String feePath = "/network/fees/transaction";
		System.out.printf(
			"Fetching recommended fees from %s%n", feePath);
		final HttpRequest feeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + feePath)).GET().build();
		final HttpResponse<String> feeResponse = HTTP_CLIENT.send(
			feeRequest, BodyHandlers.ofString());
		final JsonNode feeJSON = JSON_MAPPER.readTree(
			feeResponse.body());
		final long medianMultiplier =
			feeJSON.get("medianFeeMultiplier").asLong();
		final long minimumMultiplier =
			feeJSON.get("minFeeMultiplier").asLong();
		final long feeMultiplier = Math.max(
			medianMultiplier, minimumMultiplier);
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		// [<step-2]
		// Embedded tx 1: Account A transfers 10 XYM to Account B [>step-3]
		final EmbeddedTransaction embeddedTransaction1 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					accountBAddress,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
							// 10 XYM
							new Amount(10_000_000))),
					null),
				accountAKeyPair.getPublicKey());

		// Embedded tx 2: Account B transfers 1 custom mosaic to Account A
		final long customMosaicId = 0x6D1314BE751B62C2L;
		final EmbeddedTransaction embeddedTransaction2 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					accountAAddress,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(customMosaicId),
							// 1 custom mosaic
							new Amount(1))),
					null),
				accountBKeyPair.getPublicKey());
		// [<step-3]
		// Build the bonded aggregate transaction [>step-4]
		final List<EmbeddedTransaction> embeddedTransactions =
			List.of(embeddedTransaction1, embeddedTransaction2);
		final Transaction bondedTransaction =
			facade.createTransactionFromTypedDescriptor(
				new AggregateBondedTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(
						embeddedTransactions),
					embeddedTransactions,
					null),
				accountAKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60,
				1);
		System.out.println("Built aggregate without signatures:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(bondedTransaction.toJson()));
		// [<step-4]
		// --- ACCOUNT A (Initiator) --- [>step-5]
		// Sign the bonded aggregate transaction
		System.out.println("[Account A] Signing the bonded aggregate...");
		final CryptoTypes.Signature bondedSignature =
			facade.signTransaction(accountAKeyPair, bondedTransaction);
		final String bondedJsonPayload = SymbolTransactionFactory
			.attachSignature(bondedTransaction, bondedSignature);
		final String bondedHash = facade.hashTransaction(
			bondedTransaction).toString();
		System.out.printf("Bonded aggregate transaction hash: %s%n",
			bondedHash);
		// [<step-5]
		// Create hash lock transaction [>step-6]
		System.out.println("Creating hash lock transaction...");
		final Transaction hashLock =
			facade.createTransactionFromTypedDescriptor(
				new HashLockTransactionV1Descriptor(
					new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
						new Amount(10_000_000)), // 10 XYM deposit
					new BlockDuration(100),
					new CryptoTypes.Hash256(bondedHash)),
				accountAKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);

		// Sign hash lock
		System.out.println("[Account A] Signing the hash lock...");
		final CryptoTypes.Signature hashLockSignature =
			facade.signTransaction(accountAKeyPair, hashLock);
		final String hashLockPayload = SymbolTransactionFactory
			.attachSignature(hashLock, hashLockSignature);
		final String hashLockHash = facade.hashTransaction(hashLock)
			.toString();
		System.out.printf("Hash lock transaction hash: %s%n",
			hashLockHash);

		// Announce hash lock and wait for confirmation
		announceTransaction(
			hashLockPayload, "/transactions", "Hash lock"
		);
		waitForStatus(hashLockHash, "confirmed", "Hash lock");
		// [<step-6]
		// Announce bonded aggregate and wait for partial status [>step-7]
		announceTransaction(
			bondedJsonPayload, "/transactions/partial",
			"Bonded aggregate transaction"
		);
		waitForStatus(
			bondedHash, "partial", "Bonded aggregate transaction"
		);
		// [<step-7]
		// --- ACCOUNT B (Cosigner) --- [>step-8]
		// Retrieves partial transactions waiting for signature
		final String partialPath =
			"/transactions/partial?address=" + accountBAddress;
		System.out.println(
			"[Account B] Checking for partial transactions from "
			+ "/transactions/partial"
		);
		final HttpRequest partialRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + partialPath)).GET().build();
		final HttpResponse<String> partialResponse = HTTP_CLIENT.send(
			partialRequest, BodyHandlers.ofString());
		final JsonNode partialTxs = JSON_MAPPER.readTree(
			partialResponse.body());
		if (!partialTxs.has("data") || partialTxs.get("data").isEmpty())
			throw new IOException("No partial transactions found");

		System.out.printf("Found %d partial transaction(s)%n",
			partialTxs.get("data").size());

		// Find the transaction matching the expected hash
		boolean found = false;
		for (final JsonNode tx : partialTxs.get("data"))
				found |= bondedHash.equals(
					tx.get("meta").get("hash").asText());
		if (!found) {
			throw new IOException(
				"Expected transaction " + bondedHash
				+ " not found in partial transactions"
			);
		}
		System.out.printf("Found matching transaction: %s%n", bondedHash);
		// [<step-8]
		// Fetch full transaction details using the hash [>step-9]
		final String detailPath = "/transactions/partial/" + bondedHash;
		final HttpRequest detailRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + detailPath)).GET().build();
		final HttpResponse<String> detailResponse = HTTP_CLIENT.send(
			detailRequest, BodyHandlers.ofString());
		final JsonNode partialTxJson = JSON_MAPPER.readTree(
			detailResponse.body());

		// Verify transaction content before cosigning
		final JsonNode txData = partialTxJson.get("transaction");
		System.out.println("[Account B] Verifying transaction: "
			+ txData.get("transactions").size()
			+ " embedded transactions"
		);
		// [<step-9]
		// [>step-10]
		// Submit Account B's cosignature using the transaction hash
		final String cosignaturePath = "/transactions/cosignature";
		System.out.println(
			"[Account B] Cosigning the bonded aggregate...");
		final DetachedCosignature cosignature =
			SymbolFacade.cosignTransactionHashDetached(
				accountBKeyPair, new CryptoTypes.Hash256(bondedHash)
			);
		final String cosignaturePayload =
			JSON_MAPPER.writeValueAsString(cosignature.toJson());

		// Announce cosignature
		announceTransaction(
			cosignaturePayload, cosignaturePath, "cosignature"
		);
		// [<step-10]
		// Wait for final confirmation [>step-11]
		waitForStatus(
			bondedHash, "confirmed",
			"Bonded aggregate transaction"
		); // [<step-11]
	}
}
