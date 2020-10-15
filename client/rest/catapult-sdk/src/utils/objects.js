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

// differentiate between object, array and null even though they all report the same type ('object')
const isObject = object => 'object' === typeof object && !Array.isArray(object) && null !== object;

const areCompatible = (lhs, rhs) =>
	typeof lhs === typeof rhs && ((null === lhs) === (null === rhs)) && (Array.isArray(lhs) === Array.isArray(rhs));

/** @exports utils/objects */
const objects = {
	/**
	 * Deeply assigns properties in target from one or more source objects.
	 * @param {object} target Target object.
	 * @param {...object} sources Source objects with latter objects having precedence.
	 * @returns {object} Target object (for chaining).
	 */
	deepAssign: (target, ...sources) => {
		sources.forEach(source => Object.keys(source).forEach(key => {
			if (isObject(target[key]) && isObject(source[key]))
				objects.deepAssign(target[key], source[key]);
			else
				target[key] = source[key];
		}));

		return target;
	},

	/**
	 * Checks an object against a template and ensures that the object does not contain any properties not in the template
	 * and that all of its properties have the correct types as defined by the template.
	 * @param {object} template Template.
	 * @param {object} object Object to check against the template.
	 */
	checkSchemaAgainstTemplate: (template, object) => {
		// object can contain a subset of template properties but cannot contain any that template does not
		Object.keys(object).forEach(key => {
			if (isObject(template[key]) && isObject(object[key])) {
				objects.checkSchemaAgainstTemplate(template[key], object[key]);
			} else {
				if (undefined === template[key])
					throw new Error(`unknown '${key}' key in config`);

				if (!areCompatible(template[key], object[key]))
					throw new Error(`override '${key}' property has wrong type`);
			}
		});
	}
};

module.exports = objects;
