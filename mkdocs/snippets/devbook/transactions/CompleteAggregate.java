//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.HexFormat;
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

public final class CompleteAggregate {
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
			new CompleteAggregate().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// Account A (initiates the aggregate tx and sends XYM to [>step-1]
		// Account B)
		final String accountAPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_A_PRIVATE_KEY", "0".repeat(64));
		final KeyPair accountAKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountAPrivateKey));

		// Account B (sends custom mosaic to Account A)
		final String accountBPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_B_PRIVATE_KEY", "1".repeat(64));
		final KeyPair accountBKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountBPrivateKey));

		final Address accountAAddress = facade.network.publicKeyToAddress(
			accountAKeyPair.getPublicKey());
		final Address accountBAddress = facade.network.publicKeyToAddress(
			accountBKeyPair.getPublicKey());
		System.out.printf("Account A: %s%n", accountAAddress);
		System.out.printf("Account B: %s%n", accountBAddress); // [<step-1]

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
						new Amount(10_000_000))), // 10 XYM
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
						new Amount(1))), // 1 custom mosaic
					null),
				accountBKeyPair.getPublicKey()); // [<step-3]

		// Build the aggregate transaction [>step-4]
		final List<EmbeddedTransaction> embeddedTransactions =
			List.of(embeddedTransaction1, embeddedTransaction2);
		final AggregateCompleteTransactionV3 transaction =
			(AggregateCompleteTransactionV3) facade
				.createTransactionFromTypedDescriptor(
					new AggregateCompleteTransactionV3Descriptor(
						SymbolFacade.hashEmbeddedTransactions(
							embeddedTransactions),
						embeddedTransactions,
						null),
					accountAKeyPair.getPublicKey(),
					feeMultiplier,
					2 * 60 * 60,
					1);
		System.out.println(
			"Built aggregate transaction without signatures:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson())); // [<step-4]

		// --- ACCOUNT A (Initiator) --- [>step-5]
		System.out.println("[Account A] Signing the aggregate...");
		final CryptoTypes.Signature signatureA = facade.signTransaction(
			accountAKeyPair, transaction);
		final String transactionPayload = SymbolTransactionFactory
			.attachSignature(transaction, signatureA);
		final String payloadFormatted = JSON_MAPPER
			.writerWithDefaultPrettyPrinter()
			.writeValueAsString(JSON_MAPPER.readTree(transactionPayload));
		System.out.printf("[Account A] Payload ready to share:%n %s%n",
			payloadFormatted);

		// --- OFF-CHAIN COORDINATION ---
		// Account A sends the payload to Account B
		final String sharedPayload = transactionPayload;
		System.out.println(
			"[Account A] ==> Payload sent to Account B (offchain)");
		// [<step-5]
		// --- ACCOUNT B (Cosignatory) --- [>step-6]
		final String payloadHex = JSON_MAPPER.readTree(sharedPayload)
			.get("payload").asText();
		final Transaction receivedTransaction =
			SymbolTransactionFactory.deserialize(
				HexFormat.of().parseHex(payloadHex));

		System.out.println("[Account B] Cosigning...");
		final Cosignature cosignatureB = facade.cosignTransaction(
			accountBKeyPair, receivedTransaction);
		final String cosignatureFormatted = JSON_MAPPER
			.writerWithDefaultPrettyPrinter()
			.writeValueAsString(cosignatureB.toJson());
		System.out.printf("[Account B] Cosignature created: %s%n",
			cosignatureFormatted);

		// --- OFF-CHAIN COORDINATION ---
		// Account B sends the cosignature back to Account A
		final Cosignature sharedCosignature = cosignatureB;
		System.out.println("[Account B] <== Cosignature sent back "
			+ "to Account A (offchain)"); // [<step-6]

		// --- ACCOUNT A (Initiator) --- [>step-7]
		// Add cosignature to the transaction and rebuild payload
		transaction.getCosignatures().add(sharedCosignature);
		final String transactionPayloadFinal =
			SymbolTransactionFactory.toJson(transaction);
		final String jsonPayload = transactionPayloadFinal;
		System.out.println("[Account A] Ready to announce"); // [<step-7]

		// Announce the transaction [>step-8]
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf("Transaction hash: %s%n", transactionHash);
		announceTransaction(jsonPayload, "transaction"); // [<step-8]

		// Wait for confirmation [>step-9]
		waitForConfirmation(transactionHash, "transaction");
		// [<step-9]
	}
}
