package org.symbol.examples;

import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.AggregateCompleteTransactionV3Descriptor;
import org.symbol.sdk.symbol.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.UnresolvedMosaicDescriptor;
import org.symbol.sdk.symbol.models.Cosignature;
import org.symbol.sdk.symbol.models.EmbeddedTransaction;
import org.symbol.sdk.symbol.models.EmbeddedTransferTransactionV1;
import org.symbol.sdk.symbol.models.Transaction;
import org.symbol.sdk.utils.ArrayHelpers;

/**
 * Reads every bundled {@code part*} resource file and packages them as embedded transfer transactions inside a single Symbol
 * aggregate-complete transaction signed by the key from the {@code --private} file.
 */
public final class TransactionAggregate {
	private TransactionAggregate() {
	}

	private static List<EmbeddedTransaction> addEmbeddedTransfers(final SymbolFacade facade, final CryptoTypes.PublicKey publicKey,
			final List<Path> partFiles) {
		// obtain recipient from publicKey, so direct all transfers to 'self'
		final var recipientAddress = facade.network.publicKeyToAddress(publicKey);

		final List<EmbeddedTransaction> result = new ArrayList<>();
		for (final Path filename : partFiles) {
			final String message = ExamplesUtils.readContents(filename);
			final byte[] messageBytes = message.getBytes(StandardCharsets.UTF_8);
			// note: additional 0 byte at the beginning is added for compatibility with explorer
			// and other tools that treat messages starting with 00 byte as "plain text"
			final byte[] prefixed = ArrayHelpers.concat(new byte[1], messageBytes);

			final TransferTransactionV1Descriptor descriptor = new TransferTransactionV1Descriptor(recipientAddress, (List<UnresolvedMosaicDescriptor>) null, prefixed);
			final EmbeddedTransferTransactionV1 embeddedTransaction = (EmbeddedTransferTransactionV1) facade
					.createEmbeddedTransactionFromTypedDescriptor(descriptor, publicKey);

			System.out.println("----> " + filename.getFileName() + " length in bytes: " + embeddedTransaction.getMessage().length);
			result.add(embeddedTransaction);
		}

		return result;
	}

	public static void main(final String[] args) {
		final String privateArg = ExamplesUtils.parseFlag(args, "--private");
		if (null == privateArg)
			throw new IllegalArgumentException("missing required --private <path-to-private-key>");

		final Path privatePath = Paths.get(privateArg);

		final SymbolFacade facade = new SymbolFacade("testnet");
		final KeyPair keyPair = ExamplesUtils.readPrivateKey(privatePath);

		final List<Path> partFiles = ExamplesUtils.findBundledFiles("part", ".txt");
		final List<EmbeddedTransaction> embeddedTransactions = addEmbeddedTransfers(facade, keyPair.getPublicKey(), partFiles);
		final CryptoTypes.Hash256 merkleHash = SymbolFacade.hashEmbeddedTransactions(embeddedTransactions);

		final AggregateCompleteTransactionV3Descriptor aggregateDescriptor = new AggregateCompleteTransactionV3Descriptor(merkleHash,
				embeddedTransactions, (List<Cosignature>) null);
		// pin deterministic header values in the descriptor (like the JS / Python examples) so the output stays comparable
		final Map<String, Object> aggregateMap = aggregateDescriptor.toMap();
		aggregateMap.put("signerPublicKey", keyPair.getPublicKey());
		aggregateMap.put("fee", 0L);
		aggregateMap.put("deadline", 1L);
		final Transaction aggregateTransaction = facade.transactionFactory.create(aggregateMap);

		final CryptoTypes.Signature signature = facade.signTransaction(keyPair, aggregateTransaction);
		SymbolTransactionFactory.attachSignature(aggregateTransaction, signature);

		System.out.println("Hash: " + facade.hashTransaction(aggregateTransaction) + "\n");
		System.out.println(aggregateTransaction.toString());
	}
}
