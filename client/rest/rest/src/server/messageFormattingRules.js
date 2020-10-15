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

const catapult = require('catapult-sdk');

const { ModelType, status } = catapult.model;
const { convert, uint64 } = catapult.utils;

module.exports = {
	[ModelType.none]: value => value,
	[ModelType.binary]: value => convert.uint8ToHex(value),
	[ModelType.statusCode]: status.toString,
	[ModelType.string]: value => value.toString(),
	[ModelType.uint8]: value => value,
	[ModelType.uint16]: value => value,
	[ModelType.uint32]: value => value,
	[ModelType.uint64]: value => uint64.toString(value),
	[ModelType.uint64HexIdentifier]: value => uint64.toHex(value),
	[ModelType.int]: value => value,
	[ModelType.boolean]: value => value
};
