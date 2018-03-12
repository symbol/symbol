/** @module model/ModelFormatterBuilder */
const schemaFormatter = require('../utils/schemaFormatter');

const createFormatter = (type, modelSchema, formattingRules) => ({
	format: entity => schemaFormatter.format(entity, modelSchema[type], modelSchema, formattingRules)
});

/**
 * Builder for creating a model formatter.
 */
class ModelFormatterBuilder {
	/**
	 * Creates a model formatter builder.
	 */
	constructor() {
		this.subFormatterTypes = new Set([
			'accountWithMetadata',
			'blockHeaderWithMetadata',
			'transactionWithMetadata',

			'chainInfo',
			'storageInfo',
			'transactionStatus'
		]);
	}

	/**
	 * Adds support for a named formatter.
	 * @param {string} type The formatter type.
	 */
	addFormatter(type) {
		if (this.subFormatterTypes.has(type))
			throw Error(`formatter already registered for '${type}'`);

		this.subFormatterTypes.add(type);
	}

	/**
	 * Returns an appropriate aggregate formatter object.
	 * @param {object} modelSchema The model schema.
	 * @param {object} formattingRules A map for looking up formatting rules given a schema property type.
	 * @returns {object} The aggregate formatter object.
	 */
	build(modelSchema, formattingRules) {
		const formatter = {};
		this.subFormatterTypes.forEach(type => {
			formatter[type] = createFormatter(type, modelSchema, formattingRules);
		});

		return formatter;
	}
}

module.exports = ModelFormatterBuilder;
