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

const { sha3_256 } = require('js-sha3');

const constants = {
	namespace_base_id: [0, 0],
	namespace_max_depth: 3,
	name_pattern: /^[a-z0-9][a-z0-9-_]*$/
};

const generateId = (parentId, name) => {
	const hash = sha3_256.create();
	hash.update(Uint32Array.from(parentId).buffer);
	hash.update(name);
	const result = new Uint32Array(hash.arrayBuffer());
	return [result[0], result[1]];
};

const throwInvalidFqn = (reason, name) => {
	throw Error(`fully qualified id is invalid due to ${reason} (${name})`);
};

const findMosaicSeparatorIndex = name => {
	const mosaicSeparatorIndex = name.lastIndexOf(':');
	if (0 > mosaicSeparatorIndex)
		throwInvalidFqn('missing mosaic', name);

	if (0 === mosaicSeparatorIndex)
		throwInvalidFqn('empty part', name);

	return mosaicSeparatorIndex;
};

const extractPartName = (name, start, size) => {
	if (0 === size)
		throwInvalidFqn('empty part', name);

	const partName = name.substr(start, size);
	if (!constants.name_pattern.test(partName))
		throwInvalidFqn(`invalid part name [${partName}]`, name);

	return partName;
};

const append = (path, id, name) => {
	if (constants.namespace_max_depth === path.length)
		throwInvalidFqn('too many parts', name);

	path.push(id);
};

const split = (name, processor) => {
	let start = 0;
	for (let index = 0; index < name.length; ++index) {
		if ('.' === name[index]) {
			processor(start, index - start);
			start = index + 1;
		}
	}

	return start;
};

/** @exports model/idGenerator */
const idGenerator = {
	/**
	 * Generates a mosaic id given a unified mosaic name.
	 * @param {string} name The unified mosaic name.
	 * @returns {module:utils/uint64~uint64} The mosaic id.
	 */
	generateMosaicId: name => {
		if (0 >= name.length)
			throwInvalidFqn('having zero length', name);

		const mosaicSeparatorIndex = findMosaicSeparatorIndex(name);

		const namespaceName = name.substr(0, mosaicSeparatorIndex);
		const namespacePath = idGenerator.generateNamespacePath(namespaceName);
		const namespaceId = namespacePath[namespacePath.length - 1];

		return generateId(namespaceId, extractPartName(name, mosaicSeparatorIndex + 1, name.length - mosaicSeparatorIndex - 1));
	},

	/**
	 * Parses a unified namespace name into a path.
	 * @param {string} name The unified namespace name.
	 * @returns {array<module:utils/uint64~uint64>} The namespace path.
	 */
	generateNamespacePath: name => {
		if (0 >= name.length)
			throwInvalidFqn('having zero length', name);

		let namespaceId = constants.namespace_base_id;
		const path = [];
		const start = split(name, (substringStart, size) => {
			namespaceId = generateId(namespaceId, extractPartName(name, substringStart, size));
			append(path, namespaceId, name);
		});

		namespaceId = generateId(namespaceId, extractPartName(name, start, name.length - start));
		append(path, namespaceId, name);
		return path;
	}
};

module.exports = idGenerator;
