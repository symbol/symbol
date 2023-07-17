/* eslint-disable no-unused-vars */
import { PublicKey } from './CryptoTypes.js';
import { NetworkTimestamp, NetworkTimestampDatetimeConverter } from './NetworkTimestamp.js';
/* eslint-enable no-unused-vars */
import Ripemd160 from 'ripemd160';

const BASE32_RFC4648_ALPHABET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';

/**
 * Represents a network.
 * @template {{bytes: Uint8Array}} TAddress
 * @template {NetworkTimestamp} TNetworkTimestamp
 */
export class Network {
	/**
	 * Creates a new network with the specified name and identifier byte.
	 * @param {string} name Network name.
	 * @param {number} identifier Network identifier byte.
	 * @param {NetworkTimestampDatetimeConverter} datetimeConverter Network timestamp datetime converter associated with this network.
	 * @param {function} addressHasher Gets the primary hasher to use in the public key to address conversion.
	 * @param {function} createAddress Creates an encoded address from an address without checksum and checksum bytes.
	 * @param {AddressConstructable} AddressClass Address class associated with this network.
	 * @param {Constructable} NetworkTimestampClass Network timestamp class associated with this network.
	 */
	constructor(name, identifier, datetimeConverter, addressHasher, createAddress, AddressClass, NetworkTimestampClass) {
		/**
		 * Network name.
		 * @type string
		 */
		this.name = name;

		/**
		 * Network identifier byte.
		 * @type number
		 */
		this.identifier = identifier;

		/**
		 * Network timestamp datetime converter associated with this network.
		 * @type NetworkTimestampDatetimeConverter
		 */
		this.datetimeConverter = datetimeConverter;

		/**
		 * @private
		 */
		this._addressHasher = addressHasher;

		/**
		 * @private
		 */
		this._createAddress = createAddress;

		/**
		 * @private
		 */
		this._AddressClass = AddressClass;

		/**
		 * @private
		 */
		this._NetworkTimestampClass = NetworkTimestampClass;
	}

	/**
	 * Converts a public key to an address.
	 * @param {PublicKey} publicKey Public key to convert.
	 * @returns {TAddress} Address corresponding to the public key input.
	 */
	publicKeyToAddress(publicKey) {
		const partOneHashBuilder = this._addressHasher();
		partOneHashBuilder.update(publicKey.bytes);
		const partOneHash = partOneHashBuilder.digest();

		const partTwoHash = new Ripemd160().update(Buffer.from(partOneHash)).digest();

		const version = new Uint8Array([this.identifier, ...partTwoHash]);

		const partThreeHashBuilder = this._addressHasher();
		partThreeHashBuilder.update(version);
		const checksum = partThreeHashBuilder.digest().subarray(0, 4);

		return this._createAddress(version, checksum);
	}

	/**
	 * Checks if an address string is valid and belongs to this network.
	 * @param {string} addressString Address to check.
	 * @returns {boolean} \c true if address is valid and belongs to this network.
	 */
	isValidAddressString(addressString) {
		if (this._AddressClass.ENCODED_SIZE !== addressString.length)
			return false;

		for (let i = 0; i < addressString.length; ++i) {
			if (-1 === BASE32_RFC4648_ALPHABET.indexOf(addressString[i]))
				return false;
		}

		return this.isValidAddress(new this._AddressClass(addressString));
	}

	/**
	 * Checks if an address is valid and belongs to this network.
	 * @param {TAddress} address Address to check.
	 * @returns {boolean} \c true if address is valid and belongs to this network.
	 */
	isValidAddress(address) {
		if (address.bytes[0] !== this.identifier)
			return false;

		const hashBuilder = this._addressHasher();
		hashBuilder.update(address.bytes.subarray(0, 1 + 20));

		const checkSumFromAddress = address.bytes.subarray(1 + 20);
		const calculatedChecksum = hashBuilder.digest().subarray(0, checkSumFromAddress.length);

		for (let i = 0; i < checkSumFromAddress.length; ++i) {
			if (checkSumFromAddress[i] !== calculatedChecksum[i])
				return false;
		}

		return true;
	}

	/**
	 * Converts a network timestamp to a datetime.
	 * @param {TNetworkTimestamp} referenceNetworkTimestamp Reference network timestamp to convert.
	 * @returns {Date} Datetime representation of the reference network timestamp.
	 */
	toDatetime(referenceNetworkTimestamp) {
		return this.datetimeConverter.toDatetime(Number(referenceNetworkTimestamp.timestamp));
	}

	/**
	 * Converts a datetime to a network timestamp.
	 * @param {Date} referenceDatetime Reference datetime to convert.
	 * @returns {TNetworkTimestamp} Network timestamp representation of the reference datetime.
	 */
	fromDatetime(referenceDatetime) {
		return new this._NetworkTimestampClass(this.datetimeConverter.toDifference(referenceDatetime));
	}

	/**
	 * Returns string representation of this object.
	 * @returns {string} String representation of this object
	 */
	toString() {
		return this.name;
	}
}

/**
 * Provides utility functions for finding a network.
 */
export class NetworkLocator {
	/**
	 * Finds a network with a specified name within a list of networks.
	 * @template {Network<any, any>} TNetwork
	 * @param {Array<TNetwork>} networks List of networks to search.
	 * @param {Array<string>|string} singleOrMultipleNames Names for which to search.
	 * @returns {TNetwork} First network with a name in the supplied list.
	 */
	static findByName(networks, singleOrMultipleNames) {
		const names = Array.isArray(singleOrMultipleNames) ? singleOrMultipleNames : [singleOrMultipleNames];
		const matchingNetwork = networks.find(network => names.some(name => name === network.name));
		if (undefined === matchingNetwork)
			throw RangeError(`no network found with name '${names.join(', ')}'`);

		return matchingNetwork;
	}

	/**
	 * Finds a network with a specified identifier within a list of networks.
	 * @template {Network<any, any>} TNetwork
	 * @param {Array<TNetwork>} networks List of networks to search.
	 * @param {Array<number>|number} singleOrMultipleIdentifiers Identifiers for which to search.
	 * @returns {TNetwork} First network with an identifier in the supplied list.
	 */
	static findByIdentifier(networks, singleOrMultipleIdentifiers) {
		const identifiers = Array.isArray(singleOrMultipleIdentifiers) ? singleOrMultipleIdentifiers : [singleOrMultipleIdentifiers];
		const matchingNetwork = networks.find(network => identifiers.some(identifier => identifier === network.identifier));
		if (undefined === matchingNetwork)
			throw RangeError(`no network found with identifier '${identifiers.join(', ')}'`);

		return matchingNetwork;
	}
}

// region type declarations

/**
 * Constructable class type.
 * @class
 * @typedef {{new(...args: any[]): object}} Constructable
 */

/**
 * Address class type.
 * @class
 * @typedef {{new(...args: any[]): object, ENCODED_SIZE: number}} AddressConstructable
 */

// endregion
