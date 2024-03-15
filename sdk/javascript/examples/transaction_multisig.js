#!/usr/bin/env node

//
// Shows how to create multisig account.
//

import { PrivateKey } from '../src/index.js';
import { KeyPair, SymbolFacade, models } from '../src/symbol/index.js';

(() => {
	const createKeyPairFromPrivateKey = privateKeyString =>
		new KeyPair(new PrivateKey(privateKeyString));

	class MultisigAccountModificationSample {
		constructor() {
			this.facade = new SymbolFacade('testnet');
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
			this.facade.transactionFactory.static.attachSignature(aggregateTransaction, signature);

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

			const aggregateTransaction = this.facade.transactionFactory.create({
				type: 'aggregate_complete_transaction_v2',
				signerPublicKey: this.multisigkeyPair.publicKey,
				fee: 625n,
				deadline: 12345n,
				transactionsHash: this.facade.static.hashEmbeddedTransactions(embeddedTransactions).bytes,
				transactions: embeddedTransactions
			});

			// aggregateTransaction as models.AggregateCompleteTransactionV2
			return (/** @type {models.AggregateCompleteTransactionV2} */ (/** @type {any} */ aggregateTransaction));
		}

		addCosignatures(aggregateTransaction) {
			const transactionHash = this.facade.hashTransaction(aggregateTransaction).bytes;
			aggregateTransaction.cosignatures = this.cosignatorykeyPairs.map(keyPair => {
				const cosignature = new models.Cosignature();
				cosignature.version = 0n;
				cosignature.signerPublicKey = new models.PublicKey(keyPair.publicKey.bytes);
				cosignature.signature = new models.Signature(keyPair.sign(transactionHash).bytes);
				return cosignature;
			});
		}
	}

	const sample = new MultisigAccountModificationSample();
	sample.run();
})();
