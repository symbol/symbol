// Runnable companion to the README: shows both the typed-descriptor path (recommended)
// and the dynamic Map<String, Object> path for Symbol transactions.

package org.symbol.examples.readme;

import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.UnresolvedMosaicDescriptor;
import org.symbol.sdk.symbol.models.Amount;
import org.symbol.sdk.symbol.models.EmbeddedTransaction;
import org.symbol.sdk.symbol.models.Transaction;
import org.symbol.sdk.symbol.models.UnresolvedMosaicId;

public final class Symbol {
	private Symbol() {
	}

	// shared by the typed and JSON paths so the three construction styles keep producing the identical wire format
	private static final long FEE_MULTIPLIER = 100L;
	private static final long DEADLINE_SECONDS = 60L * 60L;

	private static void signAndPrint(final SymbolFacade facade, final Transaction transaction) {
		System.out.println("created Symbol transaction:");
		System.out.println(transaction.toString());

		final CryptoTypes.PrivateKey privateKey = new CryptoTypes.PrivateKey("EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC");
		final CryptoTypes.Signature signature = facade.signTransaction(new KeyPair(privateKey), transaction);

		final String jsonPayload = SymbolTransactionFactory.attachSignature(transaction, signature);

		System.out.println("prepared Symbol JSON payload:");
		System.out.println(jsonPayload);
		System.out.println();
	}

	private static void printEmbedded(final EmbeddedTransaction transaction) {
		System.out.println("created Symbol embedded transaction:");
		System.out.println(transaction.toString());
		System.out.println();
	}

	private static void typedDescriptorExample(final SymbolFacade facade, final CryptoTypes.PublicKey signerPublicKey) {
		System.out.println("*** EXAMPLE CONSTRUCTION FROM TYPED DESCRIPTOR ***");
		// every field is a constructor argument
		final TransferTransactionV1Descriptor typedDescriptor = new TransferTransactionV1Descriptor(
			new Address("TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I"),
			List.of(
				new UnresolvedMosaicDescriptor(new UnresolvedMosaicId(0x7CDF3B117A3C40CCL), new Amount(1000000L)),
				new UnresolvedMosaicDescriptor(new UnresolvedMosaicId(0x1F031D8D3905B931L), new Amount(5L))),
			"hello symbol".getBytes(StandardCharsets.UTF_8));

		final Transaction typedTransaction = facade.createTransactionFromTypedDescriptor(
			typedDescriptor, signerPublicKey, FEE_MULTIPLIER, DEADLINE_SECONDS);
		signAndPrint(facade, typedTransaction);

		System.out.println("*** EXAMPLE CONSTRUCTION FROM TYPED DESCRIPTOR - EMBEDDED ***");
		final EmbeddedTransaction typedEmbedded = facade.createEmbeddedTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey);
		printEmbedded(typedEmbedded);
	}

	// JSON path — same descriptor shape as a JSON document. Integral numbers parse as Integer/Long/BigInteger by
	// magnitude, so the full u64 range works as bare numbers; decimal or 0x-hex strings are also accepted.
	private static void jsonExample(final SymbolFacade facade, final CryptoTypes.PublicKey signerPublicKey) {
		System.out.println("*** EXAMPLE CONSTRUCTION FROM JSON ***");
		final String json = """
			{
				"type": "transfer_transaction_v1",
				"recipientAddress": "TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I",
				"mosaics": [{"mosaicId": "0x7CDF3B117A3C40CC", "amount": 1000000}],
				"message": "hello symbol"
			}""";
		final Transaction jsonTransaction = facade.createTransactionFromJson(json, signerPublicKey, FEE_MULTIPLIER, DEADLINE_SECONDS);
		signAndPrint(facade, jsonTransaction);

		System.out.println("*** EXAMPLE CONSTRUCTION FROM JSON - EMBEDDED ***");
		final EmbeddedTransaction jsonEmbedded = facade.createEmbeddedTransactionFromJson(json, signerPublicKey);
		printEmbedded(jsonEmbedded);
	}

	// Dynamic descriptor path — for data-driven transaction shapes; identical wire format
	private static void untypedMapExample(final SymbolFacade facade, final CryptoTypes.PublicKey signerPublicKey) {
		System.out.println("*** EXAMPLE CONSTRUCTION FROM UNTYPED MAP ***");
		final Map<String, Object> rawDescriptor = new LinkedHashMap<>();
		rawDescriptor.put("type", "transfer_transaction_v1");
		rawDescriptor.put("signerPublicKey", signerPublicKey.toString());
		rawDescriptor.put("fee", 1000000L);
		rawDescriptor.put("deadline", 41998024783L);
		rawDescriptor.put("recipientAddress", "TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I");

		final Map<String, Object> mosaic = new LinkedHashMap<>();
		mosaic.put("mosaicId", 0x7CDF3B117A3C40CCL);
		mosaic.put("amount", 1000000L);
		rawDescriptor.put("mosaics", List.of(mosaic));
		rawDescriptor.put("message", "hello symbol");

		final Transaction rawTransaction = facade.transactionFactory.create(rawDescriptor);
		signAndPrint(facade, rawTransaction);

		System.out.println("*** EXAMPLE CONSTRUCTION FROM UNTYPED MAP - EMBEDDED ***");
		final Map<String, Object> rawEmbeddedDescriptor = new LinkedHashMap<>(rawDescriptor);
		rawEmbeddedDescriptor.remove("fee");
		rawEmbeddedDescriptor.remove("deadline");
		final EmbeddedTransaction rawEmbedded = facade.transactionFactory.createEmbedded(rawEmbeddedDescriptor);
		printEmbedded(rawEmbedded);
	}

	public static void main(final String[] args) {
		final SymbolFacade facade = new SymbolFacade("testnet");
		final CryptoTypes.PublicKey signerPublicKey = new CryptoTypes.PublicKey(
			"87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8");

		typedDescriptorExample(facade, signerPublicKey);
		jsonExample(facade, signerPublicKey);
		untypedMapExample(facade, signerPublicKey);
	}
}
