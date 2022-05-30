const { ByteArray } = require('../ByteArray');
const BasicNetwork = require('../Network').Network;
const BasicNetworkTimestamp = require('../NetworkTimestamp').NetworkTimestamp;
const { NetworkTimestampDatetimeConverter } = require('../NetworkTimestamp');
const base32 = require('../utils/base32');
const { keccak_256 } = require('@noble/hashes/sha3');

/**
 * Represents a NEM network timestamp with millisecond resolution.
 */
class NetworkTimestamp extends BasicNetworkTimestamp {
	/**
	 * Adds a specified number of seconds to this timestamp.
	 * @override
	 * @param {number} count Number of seconds to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of seconds past this timestamp.
	 */
	addSeconds(count) {
		return new NetworkTimestamp(this.timestamp + BigInt(count));
	}
}

/**
 * Represents a NEM address.
 */
class Address extends ByteArray {
	static SIZE = 25;

	static ENCODED_SIZE = 40;

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
	 * @param {Date} epochTime Network epoch time.
	 */
	constructor(name, identifier, epochTime) {
		super(
			name,
			identifier,
			new NetworkTimestampDatetimeConverter(epochTime, 'seconds'),
			() => keccak_256.create(),
			(addressWithoutChecksum, checksum) => new Address(new Uint8Array([...addressWithoutChecksum, ...checksum])),
			Address,
			NetworkTimestamp
		);
	}
}

Network.MAINNET = new Network('mainnet', 0x68, new Date(Date.UTC(2015, 2, 29, 0, 6, 25)));
Network.TESTNET = new Network('testnet', 0x98, new Date(Date.UTC(2015, 2, 29, 0, 6, 25)));
Network.NETWORKS = [Network.MAINNET, Network.TESTNET];

module.exports = { Address, Network, NetworkTimestamp };
