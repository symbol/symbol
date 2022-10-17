#!/usr/bin/env node

//
// Shows how to create multisig account.
//

import symbolSdk from '../src/index.js';

(() => {
	const createKeyPairFromPrivateKey = privateKeyString =>
		new symbolSdk.symbol.KeyPair(new symbolSdk.PrivateKey(privateKeyString));

	class MultisigAccountModificationSample {
		constructor() {
			this.facade = new symbolSdk.facade.SymbolFacade('testnet');
			this.multisigkeyPair = createKeyPairFromPrivateKey('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF');
			this.cosignatorykeyPairs = [
				createKeyPairFromPrivateKey('AABBCCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899'),
				createKeyPairFromPrivateKey('BBCCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899AA'),
				createKeyPairFromPrivateKey('CCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899AABB')
			];
		}

		run() {
			// note: it's important to SIGN the transaction BEFORE adding cosignatures
			const aggregateTransaction = this.createAggregateTransaction();

			const signature = this.facade.signTransaction(this.multisigkeyPair, aggregateTransaction);
			this.facade.transactionFactory.constructor.attachSignature(aggregateTransaction, signature);

			console.log(`Hash: ${this.facade.hashTransaction(aggregateTransaction)}`);

			this.addCosignatures(aggregateTransaction);

			console.log(`Cosignatures: ${aggregateTransaction.cosignatures.length}\n`);

			console.log(aggregateTransaction.toString());
		}

		createAggregateTransaction() {
			const embeddedTransactions = [
				this.facade.transactionFactory.createEmbedded({
					type: 'multisig_account_modification_transaction_v1',
					signerPublicKey: this.multisigkeyPair.publicKey,
					minApprovalDelta: 1,
					minRemovalDelta: 1,
					addressAdditions: this.cosignatorykeyPairs.map(keyPair => this.facade.network.publicKeyToAddress(keyPair.publicKey))
				})
			];

			return this.facade.transactionFactory.create({
				type: 'aggregate_complete_transaction_v2',
				signerPublicKey: this.multisigkeyPair.publicKey,
				fee: 625n,
				deadline: 12345n,
				transactionsHash: this.facade.constructor.hashEmbeddedTransactions(embeddedTransactions).bytes,
				transactions: embeddedTransactions
			});
		}

		addCosignatures(aggregateTransaction) {
			const transactionHash = this.facade.hashTransaction(aggregateTransaction).bytes;
			aggregateTransaction.cosignatures = this.cosignatorykeyPairs.map(keyPair => {
				const cosignature = new symbolSdk.symbol.Cosignature();
				cosignature.version = 0;
				cosignature.signerPublicKey = keyPair.publicKey;
				cosignature.signature = keyPair.sign(transactionHash);
				return cosignature;
			});
		}
	}

	const sample = new MultisigAccountModificationSample();
	sample.run();
})();
