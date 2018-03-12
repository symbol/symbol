/** @module utils/schemaFormatter */
const SchemaType = require('./SchemaType');


// if 'definition' is a number, it is the type
// otherwise, it is an object with an optional type property (default type is none)
const getSchemaType = definition => ('number' === typeof definition ? definition : definition.type || SchemaType.none);

const getDefinition = (schema, key) => {
	const definition = schema[key] || {};
	const type = getSchemaType(definition);

	return {
		type,
		resultKey: definition.resultKey || key,
		schemaName: definition.schemaName
	};
};

const getSchemaName = (schemaName, entity) => ('function' === typeof schemaName ? schemaName(entity) : schemaName);

/**
 * Formatter for formatting a catapult entity.
 * @class EntityFormatter
 */

/**
 * @function format
 * @param {object} entity The entity to format.
 * @returns {object} The formatted entity.
 * @memberof module:utils/schemaFormatter~EntityFormatter
 * @instance
 */

/** @exports utils/schemaFormatter */
const schemaFormatter = {
	/**
	 * Formats an entity according to a schema and rules.
	 * @param {object} entity The entity to format.
	 * @param {object} entitySchema The schema corresponding to the entity.
	 * @param {object} schemaDictionary A map of schema names to schemas for looking up component schemas.
	 * @param {object} formattingRules A map for looking up formatting rules given a schema property type.
	 * @returns {object} The formatted entity.
	 */
	format: (entity, entitySchema, schemaDictionary, formattingRules) => {
		// set object and array rules
		formattingRules[SchemaType.object] = (value, format) => format(value);
		formattingRules[SchemaType.array] = (array, format) => array.map(value => format(value));
		formattingRules[SchemaType.dictionary] = (dictionary, format) => {
			const result = {};
			Object.keys(dictionary).forEach(key => {
				result[key] = format(dictionary[key]);
			});

			return result;
		};

		const result = {};
		Object.keys(entity).forEach(key => {
			const definition = getDefinition(entitySchema, key);
			const rule = formattingRules[definition.type];
			if (rule) {
				const schemaName = getSchemaName(definition.schemaName, entity[key]);
				result[definition.resultKey] = rule(
					entity[key],
					'number' === typeof schemaName // if schemaName is a number, interpret it as a rule id and format child parts as values
						? formattingRules[schemaName]
						: value => schemaFormatter.format(value, schemaDictionary[schemaName], schemaDictionary, formattingRules)
				);
			}
		});

		return result;
	}
};

module.exports = schemaFormatter;
