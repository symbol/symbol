package org.symbol.examples;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

import org.symbol.examples.descriptors.NemAccountKeyLink;
import org.symbol.examples.descriptors.NemMosaic;
import org.symbol.examples.descriptors.NemMultisigAccount;
import org.symbol.examples.descriptors.NemNamespace;
import org.symbol.examples.descriptors.NemTransfer;
import org.symbol.examples.descriptors.SymbolAlias;
import org.symbol.examples.descriptors.SymbolKeyLink;
import org.symbol.examples.descriptors.SymbolLock;
import org.symbol.examples.descriptors.SymbolMetadata;
import org.symbol.examples.descriptors.SymbolMosaic;
import org.symbol.examples.descriptors.SymbolNamespace;
import org.symbol.examples.descriptors.SymbolRestrictionAccount;
import org.symbol.examples.descriptors.SymbolRestrictionMosaic;
import org.symbol.examples.descriptors.SymbolTransfer;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.JsonDescriptor;
import org.symbol.sdk.facade.NemFacade;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.nem.NemTransactionFactory;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.utils.Converter;

/**
 * Creates and signs all transaction kinds from JSON descriptor documents, selecting between NEM and Symbol via
 * {@code --blockchain=nem|symbol} and looping over the providers under {@code org.symbol.examples.descriptors}.
 */
public final class TransactionSign {
	private TransactionSign() {
	}

	private interface TransactionSample {
		void process(List<String> jsonDescriptors);

		int totalProcessed();
	}

	private static final class NemSample implements TransactionSample {
		private final NemFacade facade = new NemFacade("testnet");

		private final org.symbol.sdk.nem.KeyPair keyPair = new org.symbol.sdk.nem.KeyPair(
				new CryptoTypes.PrivateKey("11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF"));

		private int total;

		@Override
		public void process(final List<String> jsonDescriptors) {
			for (final String json : jsonDescriptors) {
				// deterministic header values (deadline 12345, default fee/timestamp) keep the output comparable
				// with the JS / Python examples
				final Map<String, Object> descriptor = JsonDescriptor.parse(json);
				descriptor.put("signerPublicKey", keyPair.getPublicKey());
				descriptor.put("deadline", 12345);

				final var transaction = facade.transactionFactory.create(descriptor);
				final CryptoTypes.Signature signature = facade.signTransaction(keyPair, transaction);
				NemTransactionFactory.attachSignature(transaction, signature);

				System.out.println("Hash: " + facade.hashTransaction(transaction));
				System.out.println(transaction.toString());
				System.out.println("---- ---- ----");
			}

			total += jsonDescriptors.size();
		}

		@Override
		public int totalProcessed() {
			return total;
		}
	}

	private static final class SymbolSample implements TransactionSample {
		private final SymbolFacade facade = new SymbolFacade("testnet");

		private final org.symbol.sdk.symbol.KeyPair keyPair = new org.symbol.sdk.symbol.KeyPair(
				new CryptoTypes.PrivateKey("11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF"));

		private int total;

		@Override
		public void process(final List<String> jsonDescriptors) {
			for (final String json : jsonDescriptors) {
				// deterministic header values (fee 625, deadline 12345) keep the output comparable with the JS / Python examples
				final Map<String, Object> descriptor = JsonDescriptor.parse(json);
				descriptor.put("signerPublicKey", keyPair.getPublicKey());
				descriptor.put("fee", 625L);
				descriptor.put("deadline", 12345L);

				// descriptor strings are UTF-8 text; the secret_proof sample's proof is hex for raw bytes, so decode it at
				// the call site — the same unhexlify step the JS / Python examples perform
				if (descriptor.containsKey("proof"))
					descriptor.put("proof", Converter.hexToUint8((String) descriptor.get("proof")));

				final var transaction = facade.transactionFactory.create(descriptor);
				final CryptoTypes.Signature signature = facade.signTransaction(keyPair, transaction);
				SymbolTransactionFactory.attachSignature(transaction, signature);

				System.out.println("Hash: " + facade.hashTransaction(transaction));
				System.out.println(transaction.toString());
				System.out.println("---- ---- ----");
			}

			total += jsonDescriptors.size();
		}

		@Override
		public int totalProcessed() {
			return total;
		}
	}

	private static void runAll(final TransactionSample sample, final Map<String, Supplier<List<String>>> providers) {
		providers.forEach((name, provider) -> sample.process(provider.get()));
		System.out.println("finished processing " + sample.totalProcessed() + " descriptors");
	}

	public static void main(final String[] args) {
		String blockchain = null;
		for (int i = 0; i < args.length; ++i) {
			if ("--blockchain".equals(args[i]) && i + 1 < args.length)
				blockchain = args[++i];
			else if (args[i].startsWith("--blockchain="))
				blockchain = args[i].substring("--blockchain=".length());
		}

		if (null == blockchain || (!"nem".equals(blockchain) && !"symbol".equals(blockchain)))
			throw new IllegalArgumentException("--blockchain must be one of: nem, symbol");

		if ("nem".equals(blockchain)) {
			final Map<String, Supplier<List<String>>> providers = new LinkedHashMap<>();
			providers.put("nem_account_key_link", NemAccountKeyLink::descriptors);
			providers.put("nem_mosaic", NemMosaic::descriptors);
			providers.put("nem_multisig_account", NemMultisigAccount::descriptors);
			providers.put("nem_namespace", NemNamespace::descriptors);
			providers.put("nem_transfer", NemTransfer::descriptors);
			runAll(new NemSample(), providers);
		} else {
			final Map<String, Supplier<List<String>>> providers = new LinkedHashMap<>();
			providers.put("symbol_alias", SymbolAlias::descriptors);
			providers.put("symbol_key_link", SymbolKeyLink::descriptors);
			providers.put("symbol_lock", SymbolLock::descriptors);
			providers.put("symbol_metadata", SymbolMetadata::descriptors);
			providers.put("symbol_mosaic", SymbolMosaic::descriptors);
			providers.put("symbol_namespace", SymbolNamespace::descriptors);
			providers.put("symbol_restriction_account", SymbolRestrictionAccount::descriptors);
			providers.put("symbol_restriction_mosaic", SymbolRestrictionMosaic::descriptors);
			providers.put("symbol_transfer", SymbolTransfer::descriptors);
			runAll(new SymbolSample(), providers);
		}
	}
}
