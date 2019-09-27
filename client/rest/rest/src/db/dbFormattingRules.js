/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const { longToUint64 } = require('./dbUtils');
const catapult = require('catapult-sdk');
const { Binary } = require('mongodb');

const { ModelType, status } = catapult.model;
const { convert, uint64 } = catapult.utils;

const rawUint64ToUint64 = value => (value instanceof Binary ? uint64.fromBytes(value.buffer) : longToUint64(value));

module.exports = {
	[ModelType.none]: value => value,
	// `binary` should support both mongo binary buffers and intermediate js buffers
	[ModelType.binary]: value => (convert.uint8ToHex(value.buffer instanceof ArrayBuffer ? value : value.buffer)),
	[ModelType.objectId]: value => (undefined === value ? '' : value.toHexString().toUpperCase()),
	[ModelType.statusCode]: value => status.toString(value >>> 0),
	[ModelType.string]: value => value.toString(),
	[ModelType.uint16]: value => (value instanceof Binary ? Buffer.from(value.buffer).readInt16LE(0) : value),
	[ModelType.uint64]: value => uint64.toString(rawUint64ToUint64(value)),
	[ModelType.uint64HexIdentifier]: value => uint64.toHex(rawUint64ToUint64(value))
};
