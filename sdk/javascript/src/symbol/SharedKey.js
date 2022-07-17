const { deriveSharedKeyFactory } = require('../SharedKey');
const tweetnacl = require('tweetnacl');

const { crypto_hash } = tweetnacl.lowlevel;
const deriveSharedKeyImpl = deriveSharedKeyFactory('catapult', crypto_hash);

/**
 * Derives shared key from key pair and other party's public key.
 * @param {KeyPair} keyPair Key pair.
 * @param {PublicKey} otherPublicKey Other party's public key.
 * @returns {SharedKey256} Shared encryption key.
 */
const deriveSharedKey = (keyPair, otherPublicKey) => deriveSharedKeyImpl(keyPair.privateKey.bytes, otherPublicKey);

module.exports = { deriveSharedKey };
