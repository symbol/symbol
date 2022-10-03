import { deriveSharedKeyFactory } from '../SharedKey.js';
import tweetnacl from 'tweetnacl';

const { crypto_hash } = tweetnacl.lowlevel;
const deriveSharedKeyImpl = deriveSharedKeyFactory('catapult', crypto_hash);

/**
 * Derives shared key from key pair and other party's public key.
 * @param {KeyPair} keyPair Key pair.
 * @param {PublicKey} otherPublicKey Other party's public key.
 * @returns {SharedKey256} Shared encryption key.
 */
export const deriveSharedKey = (keyPair, otherPublicKey) => // eslint-disable-line import/prefer-default-export
	deriveSharedKeyImpl(keyPair.privateKey.bytes, otherPublicKey);
