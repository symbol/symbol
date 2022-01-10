const Ripemd160 = require('ripemd160');

/**
 * Represents a network.
 */
class Network {
	/**
	 * Creates a new network with the specified name and identifier byte.
	 * @param {string} name Network name.
	 * @param {number} identifier Network identifier byte.
	 * @param {function} addressHasher Gets the primary hasher to use in the public key to address conversion.
	 * @param {function} createAddress Creates an encoded address from an address without checksum and checksum bytes.
	 */
	constructor(name, identifier, addressHasher, createAddress) {
		this.name = name;
		this.identifier = identifier;
		this.addressHasher = addressHasher;
		this.createAddress = createAddress;
	}

	/**
	 * Converts a public key to an address.
	 * @param {PublicKey} publicKey Public key to convert.
	 * @returns {Address} Address corresponding to the public key input.
	 */
	publicKeyToAddress(publicKey) {
		const partOneHashBuilder = this.addressHasher();
		partOneHashBuilder.update(publicKey.bytes);
		const partOneHash = new Uint8Array(partOneHashBuilder.arrayBuffer());

		const partTwoHash = new Ripemd160().update(Buffer.from(partOneHash)).digest();

		const version = new Uint8Array([this.identifier, ...partTwoHash]);

		const partThreeHashBuilder = this.addressHasher();
		partThreeHashBuilder.update(version);
		const checksum = new Uint8Array(partThreeHashBuilder.arrayBuffer()).subarray(0, 4);

		return this.createAddress(version, checksum);
	}

	/**
	 * Checks if an address is valid and belongs to this network.
	 * @param {Address} address Address to check
	 * @returns {boolean} True if address is valid and belongs to this network.
	 */
	isValidAddress(address) {
		if (address.bytes[0] !== this.identifier)
			return false;

		const hashBuilder = this.addressHasher();
		hashBuilder.update(address.bytes.subarray(0, 1 + 20));

		const checkSumFromAddress = address.bytes.subarray(1 + 20);
		const calculatedChecksum = new Uint8Array(hashBuilder.arrayBuffer()).subarray(0, checkSumFromAddress.length);

		for (let i = 0; i < checkSumFromAddress.length; ++i) {
			if (checkSumFromAddress[i] !== calculatedChecksum[i])
				return false;
		}

		return true;
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
const NetworkLocator = {
	/**
	 * Finds a network with a specified name within a list of networks.
	 * @param {array<Network>} networks List of networks to search.
	 * @param {array<string>|string} singleOrMultipleNames Names for which to search.
	 * @returns {Network} First network with a name in the supplied list.
	 */
	findByName: (networks, singleOrMultipleNames) => {
		const names = Array.isArray(singleOrMultipleNames) ? singleOrMultipleNames : [singleOrMultipleNames];
		const matchingNetwork = networks.find(network => names.some(name => name === network.name));
		if (undefined === matchingNetwork)
			throw RangeError(`no network found with name '${names.join(', ')}'`);

		return matchingNetwork;
	},

	/**
	 * Finds a network with a specified identifier within a list of networks.
	 * @param {array<Network>} networks List of networks to search.
	 * @param {array<number>|number} singleOrMultipleIdentifiers Identifiers for which to search.
	 * @returns {Network} First network with an identifier in the supplied list.
	 */
	findByIdentifier: (networks, singleOrMultipleIdentifiers) => {
		const identifiers = Array.isArray(singleOrMultipleIdentifiers) ? singleOrMultipleIdentifiers : [singleOrMultipleIdentifiers];
		const matchingNetwork = networks.find(network => identifiers.some(identifier => identifier === network.identifier));
		if (undefined === matchingNetwork)
			throw RangeError(`no network found with identifier '${identifiers.join(', ')}'`);

		return matchingNetwork;
	}
};

module.exports = { Network, NetworkLocator };
