import BaseValue from './BaseValue.js';
import { Bip32 } from './Bip32.js';
import ByteArray from './ByteArray.js';
import {
	Hash256,
	PrivateKey,
	PublicKey,
	SharedKey256,
	Signature
} from './CryptoTypes.js';
import { NetworkLocator } from './Network.js';
import { hexToUint8, uint8ToHex } from './utils/converter.js';

const utils = { hexToUint8, uint8ToHex };

export {
	/**
	 * Represents a base integer.
	 * @type {typeof BaseValue}
	 */
	BaseValue,

	/**
	 * Factory of BIP32 root nodes.
	 * @type {typeof Bip32}
	 */
	Bip32,

	/**
	 * Represents a fixed size byte array.
	 * @type {typeof ByteArray}
	 */
	ByteArray,

	// region CryptoTypes

	/**
	 * Represents a 256-bit hash.
	 * @type {typeof Hash256}
	 */
	Hash256,

	/**
	 * Represents a private key.
	 * @type {typeof PrivateKey}
	 */
	PrivateKey,

	/**
	 * Represents a public key.
	 * @type {typeof PublicKey}
	 */
	PublicKey,

	/**
	 * Represents a 256-bit symmetric encryption key.
	 * @type {typeof SharedKey256}
	 */
	SharedKey256,

	/**
	 * Represents a signature.
	 * @type {typeof Signature}
	 */
	Signature,

	/**
	 * Provides utility functions for finding a network.
	 * @type {typeof NetworkLocator}
	 */
	NetworkLocator,

	/**
	 * Network independent utilities.
	 */
	utils
};
