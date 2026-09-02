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

public final class FeeSponsorship {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = "https://reference.symboltest.net:3001";

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private final KeyPair appKeyPair;

	private final KeyPair userKeyPair;

	private long feeMultiplier;

	FeeSponsorship() {
		final String appPrivateKey = System.getenv().getOrDefault(
			"APP_PRIVATE_KEY", "0".repeat(64));
		appKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(appPrivateKey));
		System.out.printf("App public key: %s%n",
			appKeyPair.getPublicKey());

		final String userPrivateKey = System.getenv().getOrDefault(
			"USER_PRIVATE_KEY", "%064X".formatted(99));
		userKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(userPrivateKey));
		System.out.printf("User public key: %s%n",
			userKeyPair.getPublicKey());
	}

	public static void main(final String[] args) {
		try {
			new FeeSponsorship().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// Fetch recommended fees
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
		feeMultiplier = Math.max(medianMultiplier, minimumMultiplier);
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);

		// Choose one
		final TransactionAndPayload result =
			buildPrefundedMessageTransaction(
				"TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I",
				"Hello world!"
			);
		// final TransactionAndPayload result =
		// 	buildSponsoredMessageTransaction(
		// 		"TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I",
		// 		"Hello world!"
		// 	);
		final Transaction transaction = result.transaction();
		final String jsonPayload = result.jsonPayload();

		System.out.println("Built transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		// Announce the transaction
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

		// Wait for confirmation
		final String statusPath =
			"/transactionStatus/" + facade.hashTransaction(transaction);
		System.out.printf(
			"Waiting for confirmation from %s%n", statusPath);

		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			try {
				final HttpRequest request = HttpRequest.newBuilder(
					URI.create(nodeUrl + statusPath)).GET().build();
				final HttpResponse<String> response = HTTP_CLIENT.send(
					request, BodyHandlers.ofString());
				if (response.statusCode() / 100 != 2)
					throw new IOException(response.toString());

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
			} catch (final IOException ex) {
				System.out.printf(
					"  Transaction status: unknown | Cause: (%s)%n",
					ex.getMessage()
				);
			}
		}
	}

	// OPTION 1 [>step-1]
	private TransactionAndPayload buildPrefundedMessageTransaction(
		final String recipientAddress,
		final String message
	) throws IOException {
		// Build the embedded message transaction [>step-2]
		final EmbeddedTransaction messageTransaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					new Address(recipientAddress),
					null,
					message.getBytes(StandardCharsets.UTF_8)),
				userKeyPair.getPublicKey());
		// [<step-2]
		// Build the embedded prefund transaction [>step-3]
		final EmbeddedTransferTransactionV1 prefundTransaction =
			(EmbeddedTransferTransactionV1) facade
				.createEmbeddedTransactionFromTypedDescriptor(
					new TransferTransactionV1Descriptor(
						facade.network.publicKeyToAddress(
							userKeyPair.getPublicKey()),
						List.of(new UnresolvedMosaicDescriptor(
							new UnresolvedMosaicId(
								IdGenerator.generateMosaicAliasId(
									"symbol.xym")),
							// To be filled once value is known.
							new Amount(0))),
						null),
					appKeyPair.getPublicKey());
		// [<step-3]
		// Build the wrapper complete aggregate transaction [>step-4]
		final List<EmbeddedTransaction> transactions =
			List.of(messageTransaction, prefundTransaction);
		final AggregateCompleteTransactionV3 transaction =
			(AggregateCompleteTransactionV3) facade
					.createTransactionFromTypedDescriptor(
						new AggregateCompleteTransactionV3Descriptor(
							SymbolFacade.hashEmbeddedTransactions(
								transactions),
							transactions,
							null),
					userKeyPair.getPublicKey(),
					feeMultiplier,
					2 * 60 * 60,
					1);
		// Update the prefund amount to match the total fee
		prefundTransaction.getMosaics().get(0).setAmount(
			transaction.getFee());
		// Update the embedded transaction hashes
		transaction.setTransactionsHash(
			Hash256.parse(SymbolFacade.hashEmbeddedTransactions(
				transactions)));
		// [<step-4]
		// [>step-5]
		// Sign the aggregate transaction using the user's signature
		SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(userKeyPair, transaction)
		);
		// Attach the app's cosignature
		transaction.getCosignatures().add(
			facade.cosignTransaction(appKeyPair, transaction)
		);
		// Obtain the payload
		final String jsonPayload =
			SymbolTransactionFactory.toJson(transaction); // [<step-5]

		return new TransactionAndPayload(transaction, jsonPayload);
	}
	// [<step-1]
	// OPTION 2 [>step-6]
	private TransactionAndPayload buildSponsoredMessageTransaction(
		final String recipientAddress,
		final String message
	) throws IOException {
		// Build the embedded message transaction [>step-7]
		final EmbeddedTransaction messageTransaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					new Address(recipientAddress),
					null,
					message.getBytes(StandardCharsets.UTF_8)),
				userKeyPair.getPublicKey());
		// [<step-7]
		// Build the embedded filler transaction [>step-8]
		final EmbeddedTransaction fillerTransaction =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					facade.network.publicKeyToAddress(
						appKeyPair.getPublicKey()),
					null,
					null),
				appKeyPair.getPublicKey());
		// [<step-8]
		// Build the wrapper complete aggregate transaction [>step-9]
		final List<EmbeddedTransaction> transactions =
			List.of(messageTransaction, fillerTransaction);
		final AggregateCompleteTransactionV3 transaction =
			(AggregateCompleteTransactionV3) facade
					.createTransactionFromTypedDescriptor(
						new AggregateCompleteTransactionV3Descriptor(
							SymbolFacade.hashEmbeddedTransactions(
								transactions),
							transactions,
							null),
					appKeyPair.getPublicKey(),
					feeMultiplier,
					2 * 60 * 60,
					1);
		// [<step-9]
		// [>step-10]
		// Sign the aggregate transaction using the app's signature
		SymbolTransactionFactory.attachSignature(
			transaction,
			facade.signTransaction(appKeyPair, transaction)
		);
		// Attach the user's cosignature
		transaction.getCosignatures().add(
			facade.cosignTransaction(userKeyPair, transaction)
		);
		// Obtain the payload
		final String jsonPayload = SymbolTransactionFactory.toJson(
			transaction);
		// [<step-10]
		return new TransactionAndPayload(transaction, jsonPayload);
	}
	// [<step-6]

	private record TransactionAndPayload(
		AggregateCompleteTransactionV3 transaction,
		String jsonPayload
	) {}
}
