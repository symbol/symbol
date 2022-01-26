const { ByteArray } = require('../ByteArray');
const BasicNetwork = require('../Network').Network;
const base32 = require('../utils/base32');
const { keccak_256 } = require('js-sha3');

/**
 * Represents a NEM address.
 */
class Address extends ByteArray {
	static SIZE = 25;

	/**
	 * Creates a NEM address.
	 * @param {Uint8Array|string|Address} address Input string, byte array or address.
	 */
	constructor(address) {
		let rawBytes = address;
		if ('string' === typeof address)
			rawBytes = base32.decode(address);
		else if (address instanceof Address)
			rawBytes = address.bytes;

		super(Address.SIZE, rawBytes);
	}

	/**
	 * Returns string representation of this object.
	 * @returns {string} String representation of this object
	 */
	toString() {
		return base32.encode(this.bytes);
	}
}

/**
 * Represents a NEM network.
 */
class Network extends BasicNetwork {
	/**
	 * Creates a new network with the specified name, identifier byte and generation hash seed.
	 * @param {string} name Network name.
	 * @param {number} identifier Network identifier byte.
	 */
	constructor(name, identifier) {
		super(
			name,
			identifier,
			() => keccak_256.create(),
			(addressWithoutChecksum, checksum) => new Address(new Uint8Array([...addressWithoutChecksum, ...checksum]))
		);
	}
}

Network.MAINNET = new Network('mainnet', 0x68);
Network.TESTNET = new Network('testnet', 0x98);
Network.NETWORKS = [Network.MAINNET, Network.TESTNET];

module.exports = { Address, Network };
