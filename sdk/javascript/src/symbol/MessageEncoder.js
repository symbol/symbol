const { KeyPair } = require('./KeyPair');
const { deriveSharedKey } = require('./SharedKey');
const { PrivateKey, PublicKey } = require('../CryptoTypes');
const { concatArrays, decodeAesGcm, encodeAesGcm } = require('../impl/CipherHelpers');
const { deepCompare } = require('../utils/arrayHelpers');

const DELEGATION_MARKER = Uint8Array.from(Buffer.from('FE2A8061577301E2', 'hex'));

const filterExceptions = (statement, exceptions) => {
	try {
		const message = statement();
		return [true, message];
	} catch (exception) {
		if (!exceptions.some(exceptionMessage => exception.message.includes(exceptionMessage)))
			throw exception;
	}

	return [false, undefined];
};

/**
 * Encrypts and encodes messages between two parties.
 */
class MessageEncoder {
	/**
	 * Creates message encoder around key pair.
	 * @param {KeyPair} keyPair Key pair.
	 */
	constructor(keyPair) {
		this.keyPair = keyPair;
	}

	/**
	 * Tries to decode encoded message, returns tuple:
	 *  * true, message - if message has been decoded and decrypted
	 *  * false, encodedMessage - otherwise
	 * @param {PublicKey} recipientPublicKey Recipient's public key.
	 * @param {Uint8Array} encodedMessage Encoded message
	 * @returns {array} Tuple containing decoded status and message.
	 */
	tryDecode(recipientPublicKey, encodedMessage) {
		if (1 === encodedMessage[0]) {
			const [result, message] = filterExceptions(
				() => decodeAesGcm(deriveSharedKey, this.keyPair, recipientPublicKey, encodedMessage.subarray(1)),
				['Unsupported state or unable to authenticate data']
			);
			if (result)
				return [true, message];
		}

		if (0xFE === encodedMessage[0] && 0 === deepCompare(DELEGATION_MARKER, encodedMessage.slice(0, 8))) {
			const ephemeralPublicKeyStart = DELEGATION_MARKER.length;
			const ephemeralPublicKeyEnd = ephemeralPublicKeyStart + PublicKey.SIZE;
			const ephemeralPublicKey = new PublicKey(encodedMessage.subarray(ephemeralPublicKeyStart, ephemeralPublicKeyEnd));

			const [result, message] = filterExceptions(
				() => decodeAesGcm(deriveSharedKey, this.keyPair, ephemeralPublicKey, encodedMessage.subarray(ephemeralPublicKeyEnd)),
				[
					'Unsupported state or unable to authenticate data',
					'invalid point'
				]
			);
			if (result)
				return [true, message];
		}

		return [false, encodedMessage];
	}

	/**
	 * Encodes message to recipient using recommended format.
	 * @param {PublicKey} recipientPublicKey Recipient public key.
	 * @param {Uint8Array} message Message to encode.
	 * @returns {Uint8Array} Encrypted and encoded message.
	 */
	encode(recipientPublicKey, message) {
		const { tag, initializationVector, cipherText } = encodeAesGcm(deriveSharedKey, this.keyPair, recipientPublicKey, message);

		return concatArrays(new Uint8Array([1]), tag, initializationVector, cipherText);
	}

	/**
	 * Encodes persistent harvesting delegation to node.
	 * @param {PublicKey} nodePublicKey Node public key.
	 * @param {KeyPair} remoteKeyPair Remote key pair.
	 * @param {KeyPair} vrfKeyPair Vrf key pair.
	 * @returns {Uint8Array} Encrypted and encoded harvesting delegation request.
	 */
	// eslint-disable-next-line class-methods-use-this
	encodePersistentHarvestingDelegation(nodePublicKey, remoteKeyPair, vrfKeyPair) {
		const ephemeralKeyPair = new KeyPair(PrivateKey.random());
		const message = concatArrays(remoteKeyPair.privateKey.bytes, vrfKeyPair.privateKey.bytes);
		const { tag, initializationVector, cipherText } = encodeAesGcm(deriveSharedKey, ephemeralKeyPair, nodePublicKey, message);

		return concatArrays(DELEGATION_MARKER, ephemeralKeyPair.publicKey.bytes, tag, initializationVector, cipherText);
	}
}

module.exports = { MessageEncoder };
