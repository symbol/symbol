//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.nio.charset.StandardCharsets;
import java.util.HexFormat;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.MessageEncoder;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.Transaction;

final class Messages {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	// Configuration
	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	// Helper function to poll for confirmed transaction
	private JsonNode retrieveConfirmedTransaction(
		final String hash,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Polling for %s confirmation...%n", label);
		int attempts = 0;
		final int maxAttempts = 60;

		while (attempts < maxAttempts) {
			try {
				final HttpRequest transactionConfirmed =
					HttpRequest.newBuilder(URI.create(
						nodeUrl + "/transactions/confirmed/" + hash))
						.GET()
						.build();
				final HttpResponse<String> response = HTTP_CLIENT.send(
					transactionConfirmed, BodyHandlers.ofString());
				if (response.statusCode() / 100 == 2) {
					System.out.printf("  %s confirmed!%n", label);
					return JSON_MAPPER.readTree(response.body());
				}
			} catch (final IOException ex) {
				// Transaction not yet confirmed
			}
			++attempts;
			Thread.sleep(2000);
		}

		throw new IOException(String.format(
			"%s not confirmed after %d attempts", label, maxAttempts));
	}

	public static void main(final String[] args) {
		try {
			new Messages().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// Set up sender and recipient accounts [>step-1]
		final String senderPrivateKeyString = System.getenv().getOrDefault(
			"SENDER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair senderKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(senderPrivateKeyString));
		final Address senderAddress = facade.network.publicKeyToAddress(
			senderKeyPair.getPublicKey());

		final String recipientPrivateKeyString = System.getenv()
			.getOrDefault("RECIPIENT_PRIVATE_KEY", "1".repeat(64));
		final KeyPair recipientKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(recipientPrivateKeyString));
		final Address recipientAddress = facade.network.publicKeyToAddress(
			recipientKeyPair.getPublicKey());

		System.out.printf("Sender address: %s%n", senderAddress);
		System.out.printf("Recipient address: %s%n%n", recipientAddress);
		// [<step-1]
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
		final long feeMultiplier = Math.max(
			medianMultiplier, minimumMultiplier);
		System.out.printf("  Fee multiplier: %d%n%n", feeMultiplier);

		// ===== PLAIN TEXT MESSAGE =====
		System.out.println("==> Sending Plain Text Message"); // [>step-2]

		// Create a plain text message
		final byte[] plainMessage =
			"Hello, Symbol!".getBytes(StandardCharsets.UTF_8);
		System.out.printf("Plain message: %s%n",
			new String(plainMessage, StandardCharsets.UTF_8));

		// Build transfer transaction with plain message
		final Transaction plainTransaction =
			facade.createTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					recipientAddress,
					null,
					plainMessage),
				senderKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-2]

		// Sign and announce the transaction
		final CryptoTypes.Signature plainSignature =
			facade.signTransaction(senderKeyPair, plainTransaction);
		final String plainJsonPayload = SymbolTransactionFactory
			.attachSignature(plainTransaction, plainSignature);
		final String plainTransactionHash = facade.hashTransaction(
			plainTransaction).toString();
		System.out.printf("Transaction hash: %s%n", plainTransactionHash);

		put("/transactions", plainJsonPayload);
		System.out.println("Plain message transaction announced\n");

		// ===== RECEIVING PLAIN TEXT MESSAGE =====
		// [>step-3]
		System.out.println("<== Receiving Plain Text Message");

		// Wait for confirmation
		final JsonNode plainTxData = retrieveConfirmedTransaction(
			plainTransactionHash, "Plain message transaction");

		// Decode plain message from confirmed transaction
		final byte[] receivedPlainMessage = HexFormat.of().parseHex(
			plainTxData.get("transaction").get("message").asText());
		System.out.printf("Received plain message: %s%n%n",
			new String(receivedPlainMessage, StandardCharsets.UTF_8));
		// [<step-3]
		// ===== ENCRYPTED MESSAGE =====
		System.out.println("==> Sending Encrypted Message"); // [>step-4]

		// Create a message encoder with sender's key pair
		final MessageEncoder senderMessageEncoder = new MessageEncoder(
			senderKeyPair);

		// Encrypt the message using recipient's public key
		final byte[] secretMessage =
			"This is a secret message!".getBytes(StandardCharsets.UTF_8);
		final byte[] encryptedPayload = senderMessageEncoder.encode(
			recipientKeyPair.getPublicKey(), secretMessage
		);
		System.out.printf("Original message: %s%n",
			new String(secretMessage, StandardCharsets.UTF_8));
		System.out.printf("Encrypted payload: %s%n",
			HexFormat.of().formatHex(encryptedPayload));

		// Build transfer transaction with encrypted message
		final Transaction encryptedTransaction =
			facade.createTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					recipientAddress,
					null,
					encryptedPayload),
				senderKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-4]

		// Sign and announce the transaction
		final CryptoTypes.Signature encryptedSignature =
			facade.signTransaction(senderKeyPair, encryptedTransaction);
		final String encryptedJsonPayload = SymbolTransactionFactory
			.attachSignature(encryptedTransaction, encryptedSignature);
		final String encryptedTransactionHash = facade.hashTransaction(
			encryptedTransaction).toString();
		System.out.printf("Transaction hash: %s%n",
			encryptedTransactionHash);

		put("/transactions", encryptedJsonPayload);
		System.out.println("Encrypted message transaction announced\n");

		// ===== RECEIVING ENCRYPTED MESSAGE =====
		System.out.println("<== Receiving Encrypted Message"); // [>step-5]

		// Wait for confirmation
		final JsonNode encryptedTxData = retrieveConfirmedTransaction(
			encryptedTransactionHash, "Encrypted message transaction");

		// Decode encrypted message using recipient's private key
		final MessageEncoder recipientMessageEncoder = new MessageEncoder(
			recipientKeyPair);
		final byte[] receivedEncryptedMessage = HexFormat.of().parseHex(
			encryptedTxData.get("transaction").get("message").asText());

		// Get sender's public key from the transaction
		final CryptoTypes.PublicKey senderPublicKeyFromTx =
			new CryptoTypes.PublicKey(encryptedTxData.get("transaction")
				.get("signerPublicKey").asText());

		final MessageEncoderResult result =
			recipientMessageEncoder.tryDecode(
				senderPublicKeyFromTx, receivedEncryptedMessage);

		if (result.isDecoded()) {
			System.out.printf("Recipient decrypted message: %s%n",
				new String((byte[]) result.message(),
					StandardCharsets.UTF_8));
		} else {
			System.out.println("Recipient failed to decrypt message");
		} // [<step-5]
	}

	private void put(
		final String path,
		final String payload
	) throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		HTTP_CLIENT.send(request, BodyHandlers.ofString());
	}
}
