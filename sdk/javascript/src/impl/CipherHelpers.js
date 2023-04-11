import { AesCbcCipher, AesGcmCipher } from '../Cipher.js';
import crypto from 'crypto';

const GCM_IV_SIZE = 12;
const CBC_IV_SIZE = 16;
const SALT_SIZE = 32;

export const concatArrays = (...arrays) => {
	const totalLength = arrays.map(buffer => buffer.length).reduce((accumulator, currentValue) => accumulator + currentValue);
	const result = new Uint8Array(totalLength);
	let targetOffset = 0;
	arrays.forEach(buffer => {
		result.set(buffer, targetOffset);
		targetOffset += buffer.length;
	});
	return result;
};

const decode = (tagSize, ivSize, encodedMessage) => ({
	tag: encodedMessage.subarray(0, tagSize),
	initializationVector: encodedMessage.subarray(tagSize, tagSize + ivSize),
	encodedMessageData: encodedMessage.subarray(tagSize + ivSize)
});

export const decodeAesGcm = (deriveSharedKey, keyPair, recipientPublicKey, encodedMessage) => {
	const { tag, initializationVector, encodedMessageData } = decode(AesGcmCipher.TAG_SIZE, GCM_IV_SIZE, encodedMessage);

	const sharedKey = deriveSharedKey(keyPair, recipientPublicKey);
	const cipher = new AesGcmCipher(sharedKey);

	return new Uint8Array(cipher.decrypt(concatArrays(encodedMessageData, tag), initializationVector));
};

export const decodeAesCbc = (deriveSharedKey, keyPair, recipientPublicKey, encodedMessage) => {
	const { tag, initializationVector, encodedMessageData } = decode(SALT_SIZE, CBC_IV_SIZE, encodedMessage);

	const sharedKey = deriveSharedKey(keyPair, recipientPublicKey, tag);
	const cipher = new AesCbcCipher(sharedKey);

	return new Uint8Array(cipher.decrypt(encodedMessageData, initializationVector));
};

export const encodeAesGcm = (deriveSharedKey, keyPair, recipientPublicKey, message) => {
	const sharedKey = deriveSharedKey(keyPair, recipientPublicKey);
	const cipher = new AesGcmCipher(sharedKey);

	const initializationVector = new Uint8Array(crypto.randomBytes(GCM_IV_SIZE));
	const cipherText = cipher.encrypt(message, initializationVector);
	const tagStartOffset = cipherText.length - AesGcmCipher.TAG_SIZE;
	const tag = cipherText.subarray(tagStartOffset);

	return { tag, initializationVector, cipherText: cipherText.subarray(0, tagStartOffset) };
};

export const encodeAesCbc = (deriveSharedKey, keyPair, recipientPublicKey, message) => {
	const salt = new Uint8Array(crypto.randomBytes(SALT_SIZE));
	const sharedKey = deriveSharedKey(keyPair, recipientPublicKey, salt);
	const cipher = new AesCbcCipher(sharedKey);

	const initializationVector = new Uint8Array(crypto.randomBytes(CBC_IV_SIZE));
	const cipherText = cipher.encrypt(message, initializationVector);

	return { salt, initializationVector, cipherText };
};
