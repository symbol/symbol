/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @module model/networkInfo */
const base32 = require('../utils/base32');
const convert = require('../utils/convert');

/**
 * Information about a catapult network.
 * @typedef {object} NetworkInfo
 * @property {numeric} id Network id.
 * @property {numeric} bytePrefix First byte of a compatible network (decoded) address.
 * @property {string} charPrefix First character of a compatible network (encoded) address.
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

const networkInfo = {
	/** @property {module:model/networkInfo~WellKnownNetworks} networks Information about well known networks. */
	networks,

	/**
	 * Finds network information given a network id.
	 * @param {numeric} id Network id.
	 * @returns {module:model/networkInfo~NetworkInfo} Network with the specified id or undefined if unknown.
	 */
	findById: id => findNetwork('id', id),

	/**
	 * Finds network information given a network address character prefix.
	 * @param {string} charPrefix Address character prefix.
	 * @returns {module:model/networkInfo~NetworkInfo} Network with the specified address character prefix
	 *          or undefined if unknown.
	 */
	findByCharPrefix: charPrefix => findNetwork('charPrefix', charPrefix)
};

module.exports = networkInfo;
