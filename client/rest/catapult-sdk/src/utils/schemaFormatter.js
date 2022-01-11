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

/** @module utils/schemaFormatter */
const SchemaType = require('./SchemaType');

// if 'definition' is a number, it is the type
// otherwise, it is an object with an optional type property (default type is undefined)
const getSchemaType = definition => ('number' === typeof definition ? definition : definition.type);

const getDefinition = (schema, key) => {
	const definition = undefined === schema[key] ? {} : schema[key];
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
 * @param {object} entity Entity to format.
 * @returns {object} Formatted entity.
 * @memberof module:utils/schemaFormatter~EntityFormatter
 * @instance
 */

/** @exports utils/schemaFormatter */
const schemaFormatter = {
	/**
	 * Formats an entity according to a schema and rules.
	 * @param {object} entity Entity to format.
	 * @param {object} entitySchema Schema corresponding to the entity.
	 * @param {object} schemaDictionary A map of schema names to schemas for looking up component schemas.
	 * @param {object} formattingRules A map for looking up formatting rules given a schema property type.
	 * @returns {object} Formatted entity.
	 */
	format: (entity, entitySchema, schemaDictionary, formattingRules) => {
		formattingRules[SchemaType.object] = (value, format) => format(value);
		formattingRules[SchemaType.dictionary] = (dictionary, format) => {
			const result = {};
			Object.keys(dictionary).forEach(key => {
				result[key] = format(dictionary[key]);
			});

			return result;
		};

		const result = {};
		Object.keys(entity).forEach(key => {
			const valueFormatter = schemaName => (
				'number' === typeof schemaName // if numeric, interpret as a rule id and format child parts as values
					? formattingRules[schemaName]
					: value => schemaFormatter.format(value, schemaDictionary[schemaName], schemaDictionary, formattingRules));

			const definition = getDefinition(entitySchema, key);
			if (definition.type === SchemaType.array) {
				result[definition.resultKey] = entity[key].map(arrayElementEntity => {
					const schemaName = getSchemaName(definition.schemaName, arrayElementEntity);
					return valueFormatter(schemaName)(arrayElementEntity);
				});
			} else {
				const rule = formattingRules[definition.type];
				// if no rule is found, the field is dropped from the entity
				if (rule) {
					const schemaName = getSchemaName(definition.schemaName, entity[key]);
					result[definition.resultKey] = rule(
						entity[key],
						valueFormatter(schemaName)
					);
				}
			}
		});

		return result;
	}
};

module.exports = schemaFormatter;
