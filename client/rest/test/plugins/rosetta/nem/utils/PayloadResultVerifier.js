import AccountIdentifier from '../../../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import SigningPayload from '../../../../../src/plugins/rosetta/openApi/model/SigningPayload.js';
import { utils } from 'symbol-sdk';
import { NemFacade } from 'symbol-sdk/nem';

export default class PayloadResultVerifier {
	constructor(timestamp) {
		this.timestampProperties = {
			timestamp,
			deadline: timestamp + (60 * 60)
		};

		this.facade = new NemFacade('testnet');
		this.transaction = undefined;
	}

	setTransfer(signerPublicKey, recipientAddress, amount, fee) {
		this.transaction = this.facade.transactionFactory.create({
			type: 'transfer_transaction_v1',
			signerPublicKey,
			...this.timestampProperties,
			fee,

			recipientAddress,
			amount
		});
	}

	setMultisigModification(signerPublicKey, metadata) {
		this.transaction = this.facade.transactionFactory.create({
			type: 'multisig_account_modification_transaction_v2',
			signerPublicKey,
			...this.timestampProperties,
			fee: 50000n * 10n,

			...metadata
		});
	}

	makeMultisig(signerPublicKey, cosignerPublicKeys) {
		this.transaction = this.facade.transactionFactory.create({
			type: 'multisig_transaction_v1',
			signerPublicKey,
			...this.timestampProperties,
			fee: this.transaction.fee,

			innerTransaction: this.transaction,

			cosignatures: cosignerPublicKeys.map(cosignerPublicKey => this.facade.transactionFactory.create({
				type: 'cosignature_v1',
				signerPublicKey: cosignerPublicKey,
				...this.timestampProperties,
				fee: 50000n * 3n
			}))
		});
	}

	makeSigningPayload(address) {
		const signingPayload = new SigningPayload(utils.uint8ToHex(this.facade.extractSigningPayload(this.transaction)));
		signingPayload.account_identifier = new AccountIdentifier(address);
		signingPayload.signature_type = 'ed25519_keccak';
		return signingPayload;
	}

	makeCosigningPayload(address) {
		const aggregateTransactionHash = this.facade.hashTransaction(this.transaction);
		const signingPayload = new SigningPayload(utils.uint8ToHex(aggregateTransactionHash.bytes));
		signingPayload.account_identifier = new AccountIdentifier(address);
		signingPayload.signature_type = 'ed25519_keccak';
		return signingPayload;
	}

	toHexString() {
		return utils.uint8ToHex(this.transaction.serialize());
	}
}
