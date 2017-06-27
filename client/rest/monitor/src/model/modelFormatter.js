import catapult from 'catapult-sdk';

const ModelType = catapult.model.ModelType;
const BinaryParser = catapult.parser.BinaryParser;
const catapultModelSystem = catapult.plugins.catapultModelSystem;
const convert = catapult.utils.convert;
const schemaFormatter = catapult.utils.schemaFormatter;
const uint64 = catapult.utils.uint64;

const system = catapultModelSystem.configure();

const formatter = (() => {
	const modelSchema = system.schema;
	return {
		format: entity => schemaFormatter.format(entity, modelSchema.blockHeader, modelSchema, {
			[ModelType.none]: value => value,
			[ModelType.binary]: value => convert.uint8ToHex(value),
			[ModelType.uint64]: value => uint64.compact(value)
		})
	};
})();

export default {
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
