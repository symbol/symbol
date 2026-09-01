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

final class CompleteAggregate {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

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

		// [>step-1]
		// Account A (initiates the aggregate tx
		// and sends XYM to Account B)
		final String accountAPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_A_PRIVATE_KEY",
			"00000000000000000000000000000000000000000000000000000000"
				+ "00000000");
		final KeyPair accountAKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountAPrivateKey));

		// Account B (sends custom mosaic to Account A)
		final String accountBPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_B_PRIVATE_KEY",
			"11111111111111111111111111111111111111111111111111111111"
				+ "11111111");
		final KeyPair accountBKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountBPrivateKey));

		final Address accountAAddress = facade.network.publicKeyToAddress(
			accountAKeyPair.getPublicKey());
		final Address accountBAddress = facade.network.publicKeyToAddress(
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
		// Reserve space for one cosignature
		// and calculate fee for the final transaction size
		System.out.println(
			"Built aggregate transaction without signatures:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));
		// [<step-4]
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
			+ "to Account A (offchain)");
		// [<step-6]
		// --- ACCOUNT A (Initiator) --- [>step-7]
		// Add cosignature to the transaction and rebuild payload
		transaction.getCosignatures().add(sharedCosignature);
		final String transactionPayloadFinal =
			SymbolTransactionFactory.toJson(transaction);
		final String jsonPayload = transactionPayloadFinal;
		System.out.println("[Account A] Ready to announce");
		// [<step-7]
		// Announce the transaction [>step-8]
		final String announcePath = "/transactions";
		System.out.printf(
			"Announcing transaction to %s%n", announcePath);
		final HttpRequest announceRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + announcePath))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(jsonPayload))
			.build();
		final HttpResponse<String> announceResponse = HTTP_CLIENT.send(
			announceRequest, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", announceResponse.body());

		// Compute hash of final transaction (with cosignatures)
		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		// [<step-8]
		// Wait for confirmation [>step-9]
		final String statusPath =
			"/transactionStatus/" + transactionHash;
		System.out.printf(
			"Waiting for confirmation from %s%n", statusPath);
		for (int attempt = 1; 60 >= attempt; ++attempt) {
			Thread.sleep(1000);
			final HttpRequest request = HttpRequest.newBuilder(
				URI.create(nodeUrl + statusPath)).GET().build();
			final HttpResponse<String> response = HTTP_CLIENT.send(
				request, BodyHandlers.ofString());
			if (response.statusCode() / 100 == 2) {
				final JsonNode status = JSON_MAPPER.readTree(
					response.body());
				System.out.printf("  Transaction status: %s%n",
					status.get("group").asText());
				if ("confirmed".equals(status.get("group").asText())) {
					System.out.printf(
						"Transaction confirmed in %d seconds%n",
						attempt);
					break;
				}
				if ("failed".equals(status.get("group").asText())) {
					System.out.printf("Transaction failed: %s%n",
						status.get("code").asText());
					break;
				}
			} else {
				System.out.printf(
					"  Transaction status: unknown | Cause: %d%n",
					response.statusCode());
			}
			if (60 == attempt)
				System.out.println("Confirmation took too long.");
		} // [<step-9]
	}
}
