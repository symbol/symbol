/** @module model/networkInfo */
import base32 from '../utils/base32';
import convert from '../utils/convert';

/**
 * Information about a catapult network.
 * @typedef {object} NetworkInfo
 * @property {numeric} id The network id.
 * @property {numeric} bytePrefix The first byte of a compatible network (decoded) address.
 * @property {string} charPrefix The first character of a compatible network (encoded) address.
 */

const networks = (function () {
	function createNetworkInfo(id) {
		return { id, bytePrefix: convert.uint8ToHex([id]), charPrefix: base32.encode(Uint8Array.of(id, 0, 0, 0, 0))[0] };
	}

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

function findNetwork(key, value) {
	for (const name of Object.keys(networks)) {
		if (value === networks[name][key])
			return networks[name];
	}

	return undefined;
}

export default {
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
