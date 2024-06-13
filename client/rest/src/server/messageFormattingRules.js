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

import catapult from '../catapult-sdk/index.js';
import { bufferToUnresolvedAddress } from '../db/dbUtils.js';
import { utils } from 'symbol-sdk';

const { ModelType, status } = catapult.model;

const stringOrFormat = (value, formatter) => ('string' === typeof value ? value : formatter(value));

export default {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => stringOrFormat(value, utils.uint8ToHex),
	[ModelType.statusCode]: status.toString,
	[ModelType.string]: value => value.toString(),
	[ModelType.uint8]: value => value,
	[ModelType.uint16]: value => value,
	[ModelType.uint32]: value => value,
	[ModelType.uint64]: value => value.toString(),
	[ModelType.uint64HexIdentifier]: value => BigInt(value).toString(16).padStart(16, '0').toUpperCase(),
	[ModelType.int]: value => value,
	[ModelType.boolean]: value => value,
	[ModelType.encodedAddress]: value => stringOrFormat(value, bufferToUnresolvedAddress)
};
