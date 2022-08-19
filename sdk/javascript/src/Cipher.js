const crypto = require('crypto');

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
class AesCbcCipher {
	/**
	 * Creates a cipher around an aes shared key.
	 * @param {SharedKey} aesKey AES shared key.
	 */
	constructor(aesKey) {
		this.key = aesKey;
	}

	/**
	 * Encrypts clear text.
	 * @param {Uint8Array} clearText Clear text to encrypt.
	 * @param {Uint8Array} iv IV bytes.
	 * @returns {Uint8Array} Cipher text.
	 */
	encrypt(clearText, iv) {
		const cipher = crypto.createCipheriv('aes-256-cbc', this.key.bytes, iv);

		const cipherText = cipher.update(clearText);
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
		const decipher = crypto.createDecipheriv('aes-256-cbc', this.key.bytes, iv);

		const clearText = decipher.update(cipherText);
		const padding = decipher.final();

		return concatArrays(clearText, padding);
	}
}

// endregion

// region AesGcmCipher

/**
 * Performs AES GCM encryption and decryption with a given key.
 */
class AesGcmCipher {
	static TAG_SIZE = 16;

	/**
	 * Creates a cipher around an aes shared key.
	 * @param {SharedKey} aesKey AES shared key.
	 */
	constructor(aesKey) {
		this.key = aesKey;
	}

	/**
	 * Encrypts clear text and appends tag to encrypted payload.
	 * @param {Uint8Array} clearText Clear text to encrypt.
	 * @param {Uint8Array} iv IV bytes.
	 * @returns {Uint8Array} Cipher text with appended tag.
	 */
	encrypt(clearText, iv) {
		const cipher = crypto.createCipheriv('aes-256-gcm', this.key.bytes, iv);

		const cipherText = cipher.update(clearText);
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
		const decipher = crypto.createDecipheriv('aes-256-gcm', this.key.bytes, iv);

		const tagStartOffset = cipherText.length - AesGcmCipher.TAG_SIZE;
		decipher.setAuthTag(cipherText.subarray(tagStartOffset));

		const clearText = decipher.update(cipherText.subarray(0, tagStartOffset));
		decipher.final(); // no padding for GCM
		return clearText;
	}
}

// endregion

module.exports = { AesCbcCipher, AesGcmCipher };
