import ByteArray from './ByteArray.js';
/* eslint-disable no-unused-vars */
import { PublicKey, SharedKey256 } from './CryptoTypes.js';
/* eslint-enable no-unused-vars */
import crypto from 'crypto';

// ReactNative Buffer polyfill only allows Buffers to be passed to Buffer.concat
// in order to support ReactNative, all Uint8Arrays must be wrapped in Buffer when calling crypto cipher APIs
const toBufferView = input => {
	const typedByteArray = input instanceof ByteArray ? input.bytes : input;
	return Buffer.from(typedByteArray.buffer, typedByteArray.byteOffset, typedByteArray.length);
};

const concatArrays = (lhs, rhs) => {
	const result = new Uint8Array(lhs.length + rhs.length);
	result.set(lhs);
	result.set(rhs, lhs.length);
	return result;
};

// region AesCbcCipher

/**
 * Performs AES CBC encryption and decryption with a given key.
 */
export class AesCbcCipher {
	/**
	 * Creates a cipher around an aes shared key.
	 * @param {SharedKey256} aesKey AES shared key.
	 */
	constructor(aesKey) {
		/**
		 * @private
		 */
		this._key = aesKey;
	}

	/**
	 * Encrypts clear text.
	 * @param {Uint8Array} clearText Clear text to encrypt.
	 * @param {Uint8Array} iv IV bytes.
	 * @returns {Uint8Array} Cipher text.
	 */
	encrypt(clearText, iv) {
		const cipher = crypto.createCipheriv('aes-256-cbc', toBufferView(this._key), toBufferView(iv));

		const cipherText = cipher.update(toBufferView(clearText));
		const padding = cipher.final();

		return concatArrays(cipherText, padding);
	}

	/**
	 * Decrypts cipher text.
	 * @param {Uint8Array} cipherText Cipher text to decrypt.
	 * @param {Uint8Array} iv IV bytes.
	 * @returns {Uint8Array} Clear text.
	 */
	decrypt(cipherText, iv) {
		const decipher = crypto.createDecipheriv('aes-256-cbc', toBufferView(this._key), toBufferView(iv));

		const clearText = decipher.update(toBufferView(cipherText));
		const padding = decipher.final();

		return concatArrays(clearText, padding);
	}
}

// endregion

// region AesGcmCipher

/**
 * Performs AES GCM encryption and decryption with a given key.
 */
export class AesGcmCipher {
	/**
	 * Byte size of GCM tag.
	 * @type number
	 */
	static TAG_SIZE = 16;

	/**
	 * Creates a cipher around an aes shared key.
	 * @param {SharedKey256} aesKey AES shared key.
	 */
	constructor(aesKey) {
		/**
		 * @private
		 */
		this._key = aesKey;
	}

	/**
	 * Encrypts clear text and appends tag to encrypted payload.
	 * @param {Uint8Array} clearText Clear text to encrypt.
	 * @param {Uint8Array} iv IV bytes.
	 * @returns {Uint8Array} Cipher text with appended tag.
	 */
	encrypt(clearText, iv) {
		const cipher = crypto.createCipheriv('aes-256-gcm', toBufferView(this._key), toBufferView(iv));

		const cipherText = cipher.update(toBufferView(clearText));
		cipher.final(); // no padding for GCM

		const tag = cipher.getAuthTag();

		return concatArrays(cipherText, tag);
	}

	/**
	 * Decrypts cipher text with appended tag.
	 * @param {Uint8Array} cipherText Cipher text with appended tag to decrypt.
	 * @param {Uint8Array} iv IV bytes.
	 * @returns {Uint8Array} Clear text.
	 */
	decrypt(cipherText, iv) {
		const decipher = crypto.createDecipheriv('aes-256-gcm', toBufferView(this._key), toBufferView(iv));

		const tagStartOffset = cipherText.length - AesGcmCipher.TAG_SIZE;
		decipher.setAuthTag(Buffer.from(cipherText.buffer, tagStartOffset));

		const clearText = decipher.update(Buffer.from(cipherText.buffer, 0, tagStartOffset));
		decipher.final(); // no padding for GCM
		return clearText;
	}
}

// endregion
