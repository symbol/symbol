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
