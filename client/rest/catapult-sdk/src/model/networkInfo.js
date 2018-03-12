/** @module model/networkInfo */
const base32 = require('../utils/base32');
const convert = require('../utils/convert');

/**
 * Information about a catapult network.
 * @typedef {object} NetworkInfo
 * @property {numeric} id The network id.
 * @property {numeric} bytePrefix The first byte of a compatible network (decoded) address.
 * @property {string} charPrefix The first character of a compatible network (encoded) address.
 */

const networks = (() => {
	const createNetworkInfo = id => ({
		id, bytePrefix: convert.uint8ToHex([id]), charPrefix: base32.encode(Uint8Array.of(id, 0, 0, 0, 0))[0]
	});

	/**
	 * Information about well known catapult networks.
	 * @typedef {object} WellKnownNetworks
	 * @property {NetworkInfo} mijin Mijin network information.
	 * @property {NetworkInfo} mijinTest Mijin test network information.
	 * @property {NetworkInfo} public Public network information.
	 * @property {NetworkInfo} publicTest Public test network information.
	 */
	return {
		mijin: createNetworkInfo(0x60),
		mijinTest: createNetworkInfo(0x90),
		public: createNetworkInfo(0x68),
		publicTest: createNetworkInfo(0x98)
	};
})();

const findNetwork = (key, value) => {
	const matchNetworkName = Object.keys(networks).find(name => value === networks[name][key]);
	return undefined === matchNetworkName ? undefined : networks[matchNetworkName];
};

module.exports = {
	/** @property {module:model/networkInfo~WellKnownNetworks} networks Information about well known networks. */
	networks,

	/**
	 * Finds network information given a network id.
	 * @param {numeric} id The network id.
	 * @returns {module:model/networkInfo~NetworkInfo} The network with the specified id or undefined if unknown.
	 */
	findById: id => findNetwork('id', id),

	/**
	 * Finds network information given a network address character prefix.
	 * @param {string} charPrefix The address character prefix.
	 * @returns {module:model/networkInfo~NetworkInfo} The network with the specified address character prefix
	 *          or undefined if unknown.
	 */
	findByCharPrefix: charPrefix => findNetwork('charPrefix', charPrefix)
};
