const { ByteArray } = require('../ByteArray');
const { Hash256 } = require('../CryptoTypes');
const BasicNetwork = require('../Network').Network;
const base32 = require('../utils/base32');
const { sha3_256 } = require('js-sha3');

/**
 * Represents a Symbol address.
 */
class Address extends ByteArray {
	static SIZE = 24;

	/**
	 * Creates a Symbol address.
	 * @param {Uint8Array|string|Address} address Input string, byte array or address.
	 */
	constructor(address) {
		let rawBytes = address;
		if ('string' === typeof address)
			rawBytes = base32.decode(`${address}A`).slice(0, -1);
		else if (address instanceof Address)
			rawBytes = address.bytes;

		super(Address.SIZE, rawBytes);
	}

	/**
	 * Returns string representation of this object.
	 * @returns {string} String representation of this object
	 */
	toString() {
		return base32.encode(new Uint8Array([...this.bytes, 0])).slice(0, -1);
	}
}

/**
 * Represents a Symbol network.
 */
class Network extends BasicNetwork {
	/**
	 * Creates a new network with the specified name, identifier byte and generation hash seed.
	 * @param {string} name Network name.
	 * @param {number} identifier Network identifier byte.
	 * @param {Hash256} generationHashSeed Network generation hash seed.
	 */
	constructor(name, identifier, generationHashSeed = undefined) {
		super(
			name,
			identifier,
			() => sha3_256.create(),
			(addressWithoutChecksum, checksum) => new Address(new Uint8Array([...addressWithoutChecksum, ...checksum.subarray(0, 3)]))
		);
		this.generationHashSeed = generationHashSeed;
	}
}

Network.MAINNET = new Network('mainnet', 0x68, new Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6'));
Network.TESTNET = new Network('testnet', 0x98, new Hash256('7FCCD304802016BEBBCD342A332F91FF1F3BB5E902988B352697BE245F48E836'));
Network.NETWORKS = [Network.MAINNET, Network.TESTNET];

module.exports = { Address, Network };
