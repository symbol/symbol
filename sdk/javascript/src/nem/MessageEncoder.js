/* eslint-disable no-unused-vars */
import { KeyPair } from './KeyPair.js';
/* eslint-enable no-unused-vars */
import { deriveSharedKey, deriveSharedKeyDeprecated } from './SharedKey.js'; // eslint-disable-line import/no-deprecated
import { Message, MessageType } from './models.js';
/* eslint-disable no-unused-vars */
import { PublicKey } from '../CryptoTypes.js';
/* eslint-enable no-unused-vars */
import {
	concatArrays, decodeAesCbc, decodeAesGcm, encodeAesCbc, encodeAesGcm
} from '../impl/CipherHelpers.js';

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
export default class MessageEncoder {
	/**
	 * Creates message encoder around key pair.
	 * @param {KeyPair} keyPair Key pair.
	 */
	constructor(keyPair) {
		/**
		 * @private
		 */
		this._keyPair = keyPair;
	}

	/**
	 * Public key used for message encoding.
	 * @returns {PublicKey} Public key used for message encoding.
	 */
	get publicKey() {
		return this._keyPair.publicKey;
	}

	/**
	 * Tries to decode encoded message.
	 * @param {PublicKey} recipientPublicKey Recipient public key.
	 * @param {Message} encodedMessage Encoded message.
	 * @returns {TryDecodeResult} Tuple containing decoded status and message.
	 */
	tryDecode(recipientPublicKey, encodedMessage) {
		if (MessageType.ENCRYPTED !== encodedMessage.messageType)
			throw new Error('invalid message format');

		let [result, message] = filterExceptions(
			() => decodeAesGcm(deriveSharedKey, this._keyPair, recipientPublicKey, encodedMessage.message),
			['Unsupported state or unable to authenticate data']
		);
		if (result)
			return { isDecoded: true, message };

		[result, message] = filterExceptions(
			// eslint-disable-next-line import/no-deprecated
			() => decodeAesCbc(deriveSharedKeyDeprecated, this._keyPair, recipientPublicKey, encodedMessage.message),
			[
				'bad decrypt',
				'wrong final block length',
				'Invalid initialization vector'
			]
		);
		if (result)
			return { isDecoded: true, message };

		return { isDecoded: false, message: encodedMessage };
	}

	/**
	 * Encodes message to recipient using recommended format.
	 * @param {PublicKey} recipientPublicKey Recipient public key.
	 * @param {Uint8Array} message Message to encode.
	 * @returns {Message} Encrypted and encoded message.
	 */
	encode(recipientPublicKey, message) {
		const { tag, initializationVector, cipherText } = encodeAesGcm(deriveSharedKey, this._keyPair, recipientPublicKey, message);

		const encodedMessage = new Message();
		encodedMessage.messageType = MessageType.ENCRYPTED;
		encodedMessage.message = concatArrays(tag, initializationVector, cipherText);
		return encodedMessage;
	}

	/**
	 * Encodes message to recipient using recommended format.
	 * @deprecated This function is only provided for compatability with older NEM messages.
	 *             Please use `encode` in any new code.
	 * @param {PublicKey} recipientPublicKey Recipient public key.
	 * @param {Uint8Array} message Message to encode.
	 * @returns {Message} Encrypted and encoded message.
	 */
	encodeDeprecated(recipientPublicKey, message) {
		// eslint-disable-next-line import/no-deprecated
		const encoded = encodeAesCbc(deriveSharedKeyDeprecated, this._keyPair, recipientPublicKey, message);

		const encodedMessage = new Message();
		encodedMessage.messageType = MessageType.ENCRYPTED;
		encodedMessage.message = concatArrays(encoded.salt, encoded.initializationVector, encoded.cipherText);
		return encodedMessage;
	}
}

// region type declarations

/**
 * Result of a try decode operation.
 * @class
 * @typedef {object} TryDecodeResult
 * @property {boolean} isDecoded \c true if message has been decoded and decrypted; \c false otherwise.
 * @property {Uint8Array|Message} message Decoded message when `isDecoded` is \c true; encoded message otherwise.
 */

// endregion
