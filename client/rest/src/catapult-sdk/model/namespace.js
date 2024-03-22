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

const convert = require('../utils/convert');
/**
 * Catapult model namespace.
 * @enum {number}
 */
const namespace = {
	/** Namespace alias type. */
	aliasType: {
		/** Mosaic alias. */
		mosaic: 1,

		/** Address alias. */
		address: 2
	},
	/**
	 * Format a namespaceId *alias* into a valid recipient field value.
	 * @param {Uint8Array} namespaceId The namespaceId
	 * @param {number} networkIdentifier network identifier serialized in the output.
	 * @returns {Uint8Array} The padded notation of the alias
	 */
	encodeNamespace(namespaceId, networkIdentifier) {
		// 0x91 | namespaceId on 8 bytes | 15 bytes 0-pad = 24 bytes
		const padded = new Uint8Array(1 + 8 + 15);
		padded.set([networkIdentifier | 0x01], 0);
		padded.set(namespaceId.reverse(), 1);
		padded.set(convert.hexToUint8('00'.repeat(15)), 9);
		return padded;
	}
};

module.exports = namespace;
