import ByteArray from '../ByteArray.js';
import { Network as BasicNetwork } from '../Network.js';
import { NetworkTimestamp as BasicNetworkTimestamp, NetworkTimestampDatetimeConverter } from '../NetworkTimestamp.js';
import base32 from '../utils/base32.js';
import { keccak_256 } from '@noble/hashes/sha3';

/**
 * Represents a NEM network timestamp with second resolution.
 */
export class NetworkTimestamp extends BasicNetworkTimestamp {
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
export class Address extends ByteArray {
	/**
	 * Byte size of raw address.
	 * @type number
	 */
	static SIZE = 25;

	/**
	 * Length of encoded address string.
	 * @type number
	 */
	static ENCODED_SIZE = 40;

	/**
	 * Creates a NEM address.
	 * @param {Uint8Array|string|Address} addressInput Input string, byte array or address.
	 */
	constructor(addressInput) {
		const extractAddressBytes = () => {
			if ('string' === typeof addressInput)
				return base32.decode(addressInput);

			if (addressInput instanceof Address)
				return addressInput.bytes;

			return addressInput;
		};

		super(Address.SIZE, extractAddressBytes());
	}

	/**
	 * Returns string representation of this object.
	 * @returns {string} String representation of this object.
	 */
	toString() {
		return base32.encode(this.bytes);
	}
}

/**
 * Represents a NEM network.
 */
export class Network extends BasicNetwork {
	/**
	 * NEM main network.
	 * @type Network
	 */
	static MAINNET;

	/**
	 * NEM test network.
	 * @type Network
	 */
	static TESTNET;

	/**
	 * NEM well known networks.
	 * @type Array<Network>
	 */
	static NETWORKS;

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
