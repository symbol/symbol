import ByteArray from '../ByteArray.js';
import { Hash256 } from '../CryptoTypes.js';
import { Network as BasicNetwork } from '../Network.js';
import { NetworkTimestamp as BasicNetworkTimestamp, NetworkTimestampDatetimeConverter } from '../NetworkTimestamp.js';
import base32 from '../utils/base32.js';
import { sha3_256 } from '@noble/hashes/sha3';

/**
 * Represents a Symbol network timestamp with millisecond resolution.
 */
export class NetworkTimestamp extends BasicNetworkTimestamp {
	/**
	 * Adds a specified number of milliseconds to this timestamp.
	 * @param {number|bigint} count Number of milliseconds to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of milliseconds past this timestamp.
	 */
	addMilliseconds(count) {
		return new NetworkTimestamp(this.timestamp + BigInt(count));
	}

	/**
	 * Adds a specified number of seconds to this timestamp.
	 * @override
	 * @param {number|bigint} count Number of seconds to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of seconds past this timestamp.
	 */
	addSeconds(count) {
		return this.addMilliseconds(1000n * BigInt(count));
	}
}

/**
 * Represents a Symbol address.
 */
export class Address extends ByteArray {
	/**
	 * Byte size of raw address.
	 * @type number
	 */
	static SIZE = 24;

	/**
	 * Length of encoded address string.
	 * @type number
	 */
	static ENCODED_SIZE = 39;

	/**
	 * Creates a Symbol address.
	 * @param {Uint8Array|string|Address} addressInput Input string, byte array or address.
	 */
	constructor(addressInput) {
		const extractAddressBytes = () => {
			if ('string' === typeof addressInput)
				return base32.decode(`${addressInput}A`).slice(0, -1);

			if (addressInput instanceof Address)
				return addressInput.bytes;

			return addressInput;
		};

		super(Address.SIZE, extractAddressBytes());
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
export class Network extends BasicNetwork {
	/**
	 * Symbol main network.
	 * @type Network
	 */
	static MAINNET;

	/**
	 * Symbol test network.
	 * @type Network
	 */
	static TESTNET;

	/**
	 * Symbol well known networks.
	 * @type Array<Network>
	 */
	static NETWORKS;

	/**
	 * Creates a new network with the specified name, identifier byte and generation hash seed.
	 * @param {string} name Network name.
	 * @param {number} identifier Network identifier byte.
	 * @param {Date} epochTime Network epoch time.
	 * @param {Hash256} generationHashSeed Network generation hash seed.
	 */
	constructor(name, identifier, epochTime, generationHashSeed) {
		super(
			name,
			identifier,
			new NetworkTimestampDatetimeConverter(epochTime, 'milliseconds'),
			() => sha3_256.create(),
			(addressWithoutChecksum, checksum) => new Address(new Uint8Array([...addressWithoutChecksum, ...checksum.subarray(0, 3)])),
			Address,
			NetworkTimestamp
		);

		/**
		 * Network generation hash seed.
		 * @type Hash256
		 */
		this.generationHashSeed = generationHashSeed;
	}
}

Network.MAINNET = new Network(
	'mainnet',
	0x68,
	new Date(Date.UTC(2021, 2, 16, 0, 6, 25)),
	new Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6')
);
Network.TESTNET = new Network(
	'testnet',
	0x98,
	new Date(Date.UTC(2022, 9, 31, 21, 7, 47)),
	new Hash256('49D6E1CE276A85B70EAFE52349AACCA389302E7A9754BCF1221E79494FC665A4')
);
Network.NETWORKS = [Network.MAINNET, Network.TESTNET];
