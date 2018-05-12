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

const catapult = require('catapult-sdk');

const { ModelType, status } = catapult.model;
const { convert } = catapult.utils;

module.exports = {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => convert.uint8ToHex(value.buffer),
	[ModelType.uint64]: value => [value.getLowBitsUnsigned(), value.getHighBits() >>> 0],
	[ModelType.objectId]: value => value.toHexString().toUpperCase(),
	[ModelType.string]: value => value.toString(),
	[ModelType.statusCode]: value => status.toString(value >>> 0)
};
