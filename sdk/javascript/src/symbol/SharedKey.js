/* eslint-disable no-unused-vars */
import { KeyPair } from './KeyPair.js';
import { PublicKey, SharedKey256 } from '../CryptoTypes.js';
/* eslint-enable no-unused-vars */
import { deriveSharedKeyFactory } from '../SharedKey.js';
import { sha512 } from '@noble/hashes/sha512';

const deriveSharedKeyImpl = deriveSharedKeyFactory('catapult', sha512);

/**
 * Derives shared key from key pair and other party's public key.
 * @param {KeyPair} keyPair Key pair.
 * @param {PublicKey} otherPublicKey Other party's public key.
 * @returns {SharedKey256} Shared encryption key.
 */
const deriveSharedKey = (keyPair, otherPublicKey) => deriveSharedKeyImpl(keyPair.privateKey.bytes, otherPublicKey);

export { deriveSharedKey }; // eslint-disable-line import/prefer-default-export
