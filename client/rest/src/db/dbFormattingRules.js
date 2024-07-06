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

import { bufferToUnresolvedAddress, longToUint64 } from './dbUtils.js';
import catapult from '../catapult-sdk/index.js';
import { Binary } from 'mongodb';
import { utils } from 'symbol-sdk';

const { ModelType, status } = catapult.model;

/**
 * Some of the formatters here may be branched depending on whether the received data comes from MongoDb or simple JavaScript. This happens
 * because those formatters take data from the database and expose it to the API, and in some uncommon cases, data is fabricated outside of
 * the database environment, and is then exposed to the API, however, the underlying types are not those of the databse. This architecture
 * couples the database and the API a great deal, and this would probably need to be decoupled into two stages/layers (database parsing, and
 * internal objects parsing). However, since the vast majority of times data is streamed directly untouched from the database to the API,
 * this has not been decoupled yet.
 */

const formatBigInt = value => value.toString(16).padStart(16, '0').toUpperCase();

export default {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => (utils.uint8ToHex(value.buffer instanceof ArrayBuffer ? value : value.buffer)),
	[ModelType.objectId]: value => (undefined === value ? '' : value.toHexString().toUpperCase()),
	[ModelType.statusCode]: value => status.toString(value >>> 0),
	[ModelType.string]: value => value.toString(),
	[ModelType.uint8]: value => value,
	// `uint16` required solely because accountRestrictions->restrictionAdditions array has uint16 provided as binary
	[ModelType.uint16]: value => (value instanceof Binary ? Buffer.from(value.buffer).readInt16LE(0) : value),
	// `uint32` might be returned by mongo as signed, so always reinterpret the underlying bytes as unsigned
	[ModelType.uint32]: value => (value & 0xFFFFFFFF) >>> 0,
	[ModelType.uint64]: value => longToUint64(value).toString(),
	// `uint64HexIdentifier` requires branching because accountRestrictions.restrictionAdditions provides bigint as binary
	[ModelType.uint64HexIdentifier]: value => {
		if (value instanceof Binary) {
			// make a copy of the Binary's buffer to ensure compatability with BigUint64Array
			const copy = Uint8Array.from(value.value());
			return formatBigInt(utils.bytesToBigInt(copy, 8));
		}

		return formatBigInt(longToUint64(value));
	},
	[ModelType.int]: value => value.valueOf(),
	[ModelType.boolean]: value => value,
	[ModelType.encodedAddress]: value => bufferToUnresolvedAddress(value)
};
