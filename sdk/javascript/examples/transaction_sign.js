#!/usr/bin/env node

//
// Shows how to create all transactions manually using TransactionFactory.
//

const symbolSdk = require('../src/index');
const yargs = require('yargs');
const path = require('path');

(() => {
	class TransactionSample {
		constructor(facade, commonFields) {
			this.facade = facade;
			this.commonFields = commonFields;

			const privateKey = new symbolSdk.CryptoTypes.PrivateKey('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF');
			this.keyPair = new this.facade.constructor.KeyPair(privateKey);
		}

		processTransactionDescriptors(transactionDescriptors) {
			transactionDescriptors.forEach(descriptor => {
				this.setCommonFields(descriptor);
				const transaction = this.facade.transactionFactory.create(descriptor);
				this.signAndPrint(transaction);
			});
		}

		setCommonFields(descriptor) {
			Object.assign(descriptor, {
				signerPublicKey: this.keyPair.publicKey,
				...this.commonFields
			});
		}

		signAndPrint(transaction) {
			const signature = this.facade.signTransaction(this.keyPair, transaction);
			this.facade.transactionFactory.constructor.attachSignature(transaction, signature);

			console.log(`Hash: ${this.facade.hashTransaction(transaction)}`);
			console.log(transaction.toString());
			console.log('---- ---- ----');
		}
	}

	const runAllTests = (sample, factoryNames) => {
		let totalDescriptorsCount = 0;
		factoryNames.forEach(factoryName => {
			// eslint-disable-next-line global-require, import/no-dynamic-require
			const { descriptorFactory } = require(path.join(__dirname, 'descriptors', `${factoryName}.js`));

			const transactionDescriptors = descriptorFactory();
			sample.processTransactionDescriptors(transactionDescriptors);
			totalDescriptorsCount += transactionDescriptors.length;
		});

		console.log(`finished processing ${totalDescriptorsCount} descriptors`);
	};

	const nemTransactionSample = new TransactionSample(new symbolSdk.facade.NemFacade('testnet'), {
		deadline: 12345
	});

	const symbolTransactionSample = new TransactionSample(new symbolSdk.facade.SymbolFacade('testnet'), {
		fee: 625n,
		deadline: 12345n
	});

	const args = yargs(process.argv.slice(2))
		.option('blockchain', {
			describe: 'blockchain to run examples against',
			choices: ['nem', 'symbol'],
			require: true
		})
		.argv;

	if ('nem' === args.blockchain) {
		runAllTests(nemTransactionSample, [
			'nem_account_key_link',
			'nem_mosaic',
			'nem_multisig_account',
			'nem_namespace',
			'nem_transfer'
		]);
	} else {
		runAllTests(symbolTransactionSample, [
			'symbol_alias',
			'symbol_key_link',
			'symbol_lock',
			'symbol_metadata',
			'symbol_mosaic',
			'symbol_namespace',
			'symbol_restriction_account',
			'symbol_restriction_mosaic',
			'symbol_transfer'
		]);
	}
})();
