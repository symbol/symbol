package org.symbol.examples;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.AggregateCompleteTransactionV3Descriptor;
import org.symbol.sdk.symbol.descriptors.MultisigAccountModificationTransactionV1Descriptor;
import org.symbol.sdk.symbol.models.AggregateCompleteTransactionV3;
import org.symbol.sdk.symbol.models.Cosignature;
import org.symbol.sdk.symbol.models.EmbeddedTransaction;

/**
 * Creates a multisig account: builds an aggregate-complete transaction wrapping a single
 * {@code multisig_account_modification_transaction_v1}, signs it with the multisig key, then attaches three cosignatures.
 */
public final class TransactionMultisig {
	private TransactionMultisig() {
	}

	private static KeyPair createKeyPair(final String privateKeyHex) {
		return new KeyPair(new CryptoTypes.PrivateKey(privateKeyHex));
	}

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private final KeyPair multisigKeyPair = createKeyPair("11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF");

	private final List<KeyPair> cosignatoryKeyPairs = List.of(
			createKeyPair("AABBCCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899"),
			createKeyPair("BBCCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899AA"),
			createKeyPair("CCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899AABB"));

	private AggregateCompleteTransactionV3 createAggregateTransaction() {
		final List<Address> addressAdditions = new ArrayList<>();
		for (final KeyPair cosigner : cosignatoryKeyPairs)
			addressAdditions.add(facade.network.publicKeyToAddress(cosigner.getPublicKey()));

		final MultisigAccountModificationTransactionV1Descriptor embeddedDescriptor = new MultisigAccountModificationTransactionV1Descriptor(
				1, 1, addressAdditions, (List<Address>) null);
		final List<EmbeddedTransaction> embeddedTransactions = List
				.of(facade.createEmbeddedTransactionFromTypedDescriptor(embeddedDescriptor, multisigKeyPair.getPublicKey()));

		final AggregateCompleteTransactionV3Descriptor aggregateDescriptor = new AggregateCompleteTransactionV3Descriptor(
				SymbolFacade.hashEmbeddedTransactions(embeddedTransactions), embeddedTransactions, (List<Cosignature>) null);

		// pin deterministic header values in the descriptor (like the JS / Python examples) so the output stays comparable
		final Map<String, Object> aggregateMap = aggregateDescriptor.toMap();
		aggregateMap.put("signerPublicKey", multisigKeyPair.getPublicKey());
		aggregateMap.put("fee", 625L);
		aggregateMap.put("deadline", 12345L);
		return (AggregateCompleteTransactionV3) facade.transactionFactory.create(aggregateMap);
	}

	private void addCosignatures(final AggregateCompleteTransactionV3 aggregateTransaction) {
		final List<Cosignature> cosignatures = new ArrayList<>();
		for (final KeyPair keyPair : cosignatoryKeyPairs)
			cosignatures.add(facade.cosignTransaction(keyPair, aggregateTransaction));

		aggregateTransaction.setCosignatures(cosignatures);
	}

	private void run() {
		// note: it's important to SIGN the transaction BEFORE adding cosignatures
		final AggregateCompleteTransactionV3 aggregateTransaction = createAggregateTransaction();

		final CryptoTypes.Signature signature = facade.signTransaction(multisigKeyPair, aggregateTransaction);
		SymbolTransactionFactory.attachSignature(aggregateTransaction, signature);

		System.out.println("Hash: " + facade.hashTransaction(aggregateTransaction));

		addCosignatures(aggregateTransaction);

		System.out.println("Cosignatures: " + aggregateTransaction.getCosignatures().size() + "\n");
		System.out.println(aggregateTransaction.toString());
	}

	public static void main(final String[] args) {
		new TransactionMultisig().run();
	}
}
