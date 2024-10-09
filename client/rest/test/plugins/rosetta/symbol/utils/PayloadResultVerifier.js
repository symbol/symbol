import { createRosettaAggregateSignerKeyPair } from './rosettaTestUtils.js';
import AccountIdentifier from '../../../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import SigningPayload from '../../../../../src/plugins/rosetta/openApi/model/SigningPayload.js';
import { utils } from 'symbol-sdk';
import { SymbolFacade, generateMosaicAliasId, models } from 'symbol-sdk/symbol';

export default class PayloadResultVerifier {
	constructor() {
		this.facade = new SymbolFacade('testnet');
		this.embeddedTransactions = [];
		this.aggregateTransaction = undefined;
	}

	addTransfer(signerPublicKey, recipientAddress, amount, mosaicName = 'symbol.xym') {
		this.embeddedTransactions.push(this.facade.transactionFactory.createEmbedded({
			type: 'transfer_transaction_v1',
			signerPublicKey,
			recipientAddress,
			mosaics: [
				{ mosaicId: generateMosaicAliasId(mosaicName), amount }
			]
		}));
	}

	addMultisigModification(signerPublicKey, metadata) {
		this.embeddedTransactions.push(this.facade.transactionFactory.createEmbedded({
			type: 'multisig_account_modification_transaction_v1',
			signerPublicKey,
			...metadata
		}));
	}

	addUnsupported(signerPublicKey) {
		this.embeddedTransactions.push(this.facade.transactionFactory.createEmbedded({
			type: 'mosaic_definition_transaction_v1',
			signerPublicKey,
			duration: 1n,
			nonce: 123,
			flags: 'transferable restrictable',
			divisibility: 2
		}));
	}

	buildAggregate(signerPublicKey, deadline, cosignatureCount = 0) {
		const merkleHash = this.facade.static.hashEmbeddedTransactions(this.embeddedTransactions);
		this.aggregateTransaction = this.facade.transactionFactory.create({
			type: 'aggregate_complete_transaction_v2',
			signerPublicKey,
			deadline,
			transactionsHash: merkleHash,
			transactions: this.embeddedTransactions
		});

		const transactionSize = this.aggregateTransaction.size + (104 * cosignatureCount);
		this.aggregateTransaction.fee = new models.Amount(transactionSize * 102);
	}

	addCosignature(cosignerPublicKey) {
		const cosignature = new models.Cosignature();
		cosignature.version = 0n;
		cosignature.signerPublicKey = new models.PublicKey(cosignerPublicKey);
		cosignature.signature = new models.Signature(new Uint8Array(64));
		this.aggregateTransaction.cosignatures.push(cosignature);
	}

	setAggregateFeePayerSignature() {
		const signature = this.facade.signTransaction(createRosettaAggregateSignerKeyPair(), this.aggregateTransaction);
		this.facade.transactionFactory.static.attachSignature(this.aggregateTransaction, signature);
	}

	makeSigningPayload(address) {
		const signingPayload = new SigningPayload(utils.uint8ToHex(this.facade.extractSigningPayload(this.aggregateTransaction)));
		signingPayload.account_identifier = new AccountIdentifier(address);
		signingPayload.signature_type = 'ed25519';
		return signingPayload;
	}

	makeCosigningPayload(address) {
		const aggregateTransactionHash = this.facade.hashTransaction(this.aggregateTransaction);
		const signingPayload = new SigningPayload(utils.uint8ToHex(aggregateTransactionHash.bytes));
		signingPayload.account_identifier = new AccountIdentifier(address);
		signingPayload.signature_type = 'ed25519';
		return signingPayload;
	}

	toHexString() {
		return utils.uint8ToHex(this.aggregateTransaction.serialize());
	}
}
