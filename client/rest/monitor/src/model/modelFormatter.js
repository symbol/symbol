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

const { ModelType } = catapult.model;
const { BinaryParser } = catapult.parser;
const { catapultModelSystem } = catapult.plugins;
const { convert, schemaFormatter, uint64 } = catapult.utils;

const system = catapultModelSystem.configure([], {});

const formatter = (() => {
	// use schemaFormatter explicitly because blockHeader is not a top-level formatter
	const modelSchema = system.schema;
	return {
		format: entity => schemaFormatter.format(entity, modelSchema.blockHeader, modelSchema, {
			[ModelType.none]: value => value,
			[ModelType.binary]: value => convert.uint8ToHex(value),
			[ModelType.uint64]: value => uint64.compact(value)
		})
	};
})();

module.exports = {
	parseAndFormatBlock: buffer => {
		const parser = new BinaryParser();
		parser.push(buffer);
		const initialNumUnprocessedBytes = parser.numUnprocessedBytes();

		const rawBlock = system.codec.deserialize(parser);
		rawBlock.size = initialNumUnprocessedBytes - parser.numUnprocessedBytes();
		delete rawBlock.transactions;

		return formatter.format(rawBlock);
	}
};
