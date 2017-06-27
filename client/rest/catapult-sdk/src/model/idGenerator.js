import { sha3_256 } from 'js-sha3';

const constants = {
	namespace_base_id: [0, 0],
	namespace_max_depth: 3,
	name_pattern: /^[a-z0-9][a-z0-9-_]*$/
};

function generateId(parentId, name) {
	const hash = sha3_256.create();
	hash.update(Uint32Array.from(parentId).buffer);
	hash.update(name);
	const result = new Uint32Array(hash.arrayBuffer());
	return [result[0], result[1]];
}

function throwInvalidFqn(reason, name) {
	throw Error(`fully qualified id is invalid due to ${reason} (${name})`);
}

function findMosaicSeparatorIndex(name) {
	const mosaicSeparatorIndex = name.lastIndexOf(':');
	if (0 > mosaicSeparatorIndex)
		throwInvalidFqn('missing mosaic', name);

	if (0 === mosaicSeparatorIndex)
		throwInvalidFqn('empty part', name);

	return mosaicSeparatorIndex;
}

function extractPartName(name, start, size) {
	if (0 === size)
		throwInvalidFqn('empty part', name);

	const partName = name.substr(start, size);
	if (!constants.name_pattern.test(partName))
		throwInvalidFqn(`invalid part name [${partName}]`, name);

	return partName;
}

function append(path, id, name) {
	if (constants.namespace_max_depth === path.length)
		throwInvalidFqn('too many parts', name);

	path.push(id);
}

function split(name, processor) {
	let start = 0;
	for (let index = 0; index < name.length; ++index) {
		if ('.' === name[index]) {
			processor(start, index - start);
			start = index + 1;
		}
	}

	return start;
}

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

		return generateId(
				namespaceId,
				extractPartName(name, mosaicSeparatorIndex + 1, name.length - mosaicSeparatorIndex - 1));
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

export default idGenerator;
