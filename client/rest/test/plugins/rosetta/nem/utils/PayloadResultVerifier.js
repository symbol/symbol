import AccountIdentifier from '../../../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import SigningPayload from '../../../../../src/plugins/rosetta/openApi/model/SigningPayload.js';
import { utils } from 'symbol-sdk';
import { NemFacade, models } from 'symbol-sdk/nem';

const FEE_UNIT = 50000n;

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

	setTransferWithArbitraryMosaic(signerPublicKey, recipientAddress) {
		const textEncoder = new TextEncoder();
		this.transaction = this.facade.transactionFactory.create({
			type: 'transfer_transaction_v2',
			signerPublicKey,
			...this.timestampProperties,
			fee: 25n * FEE_UNIT,

			recipientAddress,
			amount: 2_000000,
			mosaics: [
				{
					mosaic: {
						mosaicId: { namespaceId: { name: textEncoder.encode('nem') }, name: textEncoder.encode('xem') },
						amount: 12345_000000
					}
				},
				{
					mosaic: {
						mosaicId: { namespaceId: { name: textEncoder.encode('foo') }, name: textEncoder.encode('bar') },
						amount: 54321_000000
					}
				}
			]
		});
	}

	setMultisigModification(signerPublicKey, metadata) {
		this.transaction = this.facade.transactionFactory.create({
			type: 'multisig_account_modification_transaction_v2',
			signerPublicKey,
			...this.timestampProperties,
			fee: 10n * FEE_UNIT,

			...metadata
		});
	}

	makeMultisig(signerPublicKey, cosignerPublicKeys) {
		const otherTransactionHash = this.facade.hashTransaction(this.transaction);
		const multisigAccountAddress = this.facade.network.publicKeyToAddress(this.transaction.signerPublicKey);

		this.transaction = this.facade.transactionFactory.create({
			type: 'multisig_transaction_v1',
			signerPublicKey,
			...this.timestampProperties,
			fee: 3n * FEE_UNIT,

			innerTransaction: this.facade.transactionFactory.static.toNonVerifiableTransaction(this.transaction),

			cosignatures: cosignerPublicKeys.map(cosignerPublicKey => {
				const cosignature = new models.SizePrefixedCosignatureV1();
				cosignature.cosignature = this.facade.transactionFactory.create({
					type: 'cosignature_v1',
					signerPublicKey: cosignerPublicKey,
					...this.timestampProperties,
					fee: 3n * FEE_UNIT,
					otherTransactionHash,
					multisigAccountAddress
				});
				return cosignature;
			})
		});
	}

	makeSigningPayload(address) {
		const signingPayload = new SigningPayload(utils.uint8ToHex(this.facade.extractSigningPayload(this.transaction)));
		signingPayload.account_identifier = new AccountIdentifier(address);
		signingPayload.signature_type = 'ed25519_keccak';
		return signingPayload;
	}

	findCosignature(address) {
		return this.transaction.cosignatures.map(cosignature => cosignature.cosignature)
			.find(cosignature => this.facade.network.publicKeyToAddress(cosignature.signerPublicKey).toString() === address);
	}

	makeCosigningPayload(address) {
		const cosignature = this.findCosignature(address);
		const signingPayload = new SigningPayload(utils.uint8ToHex(this.facade.extractSigningPayload(cosignature)));
		signingPayload.account_identifier = new AccountIdentifier(address);
		signingPayload.signature_type = 'ed25519_keccak';
		return signingPayload;
	}

	toHexString() {
		return utils.uint8ToHex(this.transaction.serialize());
	}
}
