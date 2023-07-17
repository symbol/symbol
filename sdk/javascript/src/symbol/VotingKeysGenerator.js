import { KeyPair } from './KeyPair.js';
import { PrivateKey } from '../CryptoTypes.js';

const setBuffer = (destination, offset, source) => {
	source.forEach((byte, i) => { destination.setUint8(offset + i, source[i]); });
};

/**
 * Generates symbol voting keys.
 */
export default class VotingKeysGenerator {
	/**
	 * Creates a generator around a voting root key pair.
	 * @param {KeyPair} rootKeyPair Voting root key pair.
	 * @param {function} privateKeyGenerator Private key generator.
	 */
	constructor(rootKeyPair, privateKeyGenerator = PrivateKey.random) {
		/**
		 * @private
		 */
		this._rootKeyPair = rootKeyPair;

		/**
		 * @private
		 */
		this._privateKeyGenerator = privateKeyGenerator;
	}

	/**
	 * Generates voting keys for specified epochs.
	 * @param {bigint} startEpoch Start epoch.
	 * @param {bigint} endEpoch End epoch.
	 * @returns {Uint8Array} Serialized voting keys.
	 */
	generate(startEpoch, endEpoch) {
		const HEADER_SIZE = 80;
		const EPOCH_ENTRY_SIZE = 96;

		const numEpochs = Number(endEpoch - startEpoch + 1n);
		const buffer = new ArrayBuffer(HEADER_SIZE + (EPOCH_ENTRY_SIZE * numEpochs));

		const view = new DataView(buffer);
		view.setBigUint64(0, startEpoch, true); // start key identifier
		view.setBigUint64(8, endEpoch, true); // end key identifier
		view.setBigUint64(16, 0xFFFFFFFFFFFFFFFFn, true); // reserved - last (used) key identifier
		view.setBigUint64(24, 0xFFFFFFFFFFFFFFFFn, true); // reserved - last wiped key identifier

		setBuffer(view, 32, this._rootKeyPair.publicKey.bytes); // root voting public key
		view.setBigUint64(64, startEpoch, true); // level 1/1 start key identifier
		view.setBigUint64(72, endEpoch, true); // level 1/1 end key identifier

		for (let i = 0; i < numEpochs; ++i) {
			const identifier = endEpoch - BigInt(i);
			const childPrivateKey = this._privateKeyGenerator();
			const childKeyPair = new KeyPair(childPrivateKey);

			const parentSignedPayloadBuffer = new ArrayBuffer(40);
			const parentSignedPayloadView = new DataView(parentSignedPayloadBuffer);
			setBuffer(parentSignedPayloadView, 0, childKeyPair.publicKey.bytes);
			parentSignedPayloadView.setBigUint64(32, identifier, true);
			const signature = this._rootKeyPair.sign(new Uint8Array(parentSignedPayloadBuffer));

			const startOffset = HEADER_SIZE + (EPOCH_ENTRY_SIZE * i);
			setBuffer(view, startOffset, childKeyPair.privateKey.bytes); // child voting private key used to sign votes for an epoch
			setBuffer(view, startOffset + PrivateKey.SIZE, signature.bytes); // signature proving derivation of child key pair from root
		}

		return new Uint8Array(buffer);
	}
}
