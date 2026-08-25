// Runnable companion to the README: shows both the typed-descriptor path (recommended)
// and the dynamic Map<String, Object> path for NEM transactions.

package org.symbol.examples.readme;

import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.Map;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.NemFacade;
import org.symbol.sdk.nem.Address;
import org.symbol.sdk.nem.KeyPair;
import org.symbol.sdk.nem.NemTransactionFactory;
import org.symbol.sdk.nem.descriptors.MessageDescriptor;
import org.symbol.sdk.nem.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.nem.models.Amount;
import org.symbol.sdk.nem.models.MessageType;
import org.symbol.sdk.nem.models.Transaction;

public final class Nem {
	private Nem() {
	}

	private static void signAndPrint(final NemFacade facade, final Transaction transaction) {
		System.out.println("created NEM transaction:");
		System.out.println(transaction.toString());

		final CryptoTypes.PrivateKey privateKey = new CryptoTypes.PrivateKey("EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC");
		final CryptoTypes.Signature signature = facade.signTransaction(new KeyPair(privateKey), transaction);

		final String jsonPayload = NemTransactionFactory.attachSignature(transaction, signature);

		System.out.println("prepared NEM JSON payload:");
		System.out.println(jsonPayload);
		System.out.println();
	}

	private static void typedDescriptorExample(final NemFacade facade, final CryptoTypes.PublicKey signerPublicKey) {
		System.out.println("*** EXAMPLE CONSTRUCTION FROM TYPED DESCRIPTOR ***");
		// every field is a constructor argument
		final TransferTransactionV1Descriptor typedDescriptor = new TransferTransactionV1Descriptor(
			new Address("TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW"), new Amount(5100000L),
			new MessageDescriptor(MessageType.PLAIN, "hello nem".getBytes(StandardCharsets.UTF_8)));

		final long fee = 0x186A0L;
		final long deadlineSeconds = 60L * 60L * 24L;
		final Transaction typedTransaction = facade.createTransactionFromTypedDescriptor(
			typedDescriptor, signerPublicKey, fee, deadlineSeconds);
		signAndPrint(facade, typedTransaction);
	}

	// JSON path — same descriptor shape as a JSON document, nested message descriptor included.
	// The message type accepts its enum name; integral numbers parse as Integer/Long/BigInteger by magnitude.
	private static void jsonExample(final NemFacade facade, final CryptoTypes.PublicKey signerPublicKey) {
		System.out.println("*** EXAMPLE CONSTRUCTION FROM JSON ***");
		final String json = """
			{
				"type": "transfer_transaction_v1",
				"recipientAddress": "TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW",
				"amount": 5100000,
				"message": {"messageType": "plain", "message": "hello nem"}
			}""";
		final long fee = 0x186A0L;
		final long deadlineSeconds = 60L * 60L * 24L;
		final Transaction jsonTransaction = facade.createTransactionFromJson(json, signerPublicKey, fee, deadlineSeconds);
		signAndPrint(facade, jsonTransaction);
	}

	// Dynamic descriptor path — useful when the transaction shape is built from data
	// (forms, JSON, etc.) rather than known at compile time.
	private static void untypedMapExample(final NemFacade facade, final CryptoTypes.PublicKey signerPublicKey) {
		System.out.println("*** EXAMPLE CONSTRUCTION FROM UNTYPED MAP ***");
		final Map<String, Object> rawDescriptor = new LinkedHashMap<>();
		rawDescriptor.put("type", "transfer_transaction_v1");
		rawDescriptor.put("signerPublicKey", signerPublicKey.toString());
		rawDescriptor.put("fee", 0x186A0L);
		rawDescriptor.put("timestamp", 191205516L);
		rawDescriptor.put("deadline", 191291916L);
		rawDescriptor.put("recipientAddress", "TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW");
		rawDescriptor.put("amount", 5100000L);

		final Map<String, Object> message = new LinkedHashMap<>();
		message.put("messageType", 1);
		message.put("message", "hello nem");
		rawDescriptor.put("message", message);

		final Transaction rawTransaction = facade.transactionFactory.create(rawDescriptor);
		signAndPrint(facade, rawTransaction);
	}

	public static void main(final String[] args) {
		final NemFacade facade = new NemFacade("testnet");
		final CryptoTypes.PublicKey signerPublicKey = new CryptoTypes.PublicKey(
			"A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74");

		typedDescriptorExample(facade, signerPublicKey);
		jsonExample(facade, signerPublicKey);
		untypedMapExample(facade, signerPublicKey);
	}
}
