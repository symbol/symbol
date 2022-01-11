const { sha3_256 } = require('js-sha3');

const NAMESPACE_FLAG = BigInt(1) << BigInt(63);

const uint32ToBytes = value => new Uint8Array([
	value & 0xFF,
	(value >> 8) & 0xFF,
	(value >> 16) & 0xFF,
	(value >> 24) & 0xFF
]);

const digestToBigInt = digest => {
	let result = BigInt(0);
	for (let i = 0; 8 > i; ++i)
		result += (BigInt(digest[i]) << BigInt(8 * i));

	return result;
};

/**
 * Generates a mosaic id from an owner address and a nonce.
 * @param {Address} ownerAddress Owner address.
 * @param {number} nonce Nonce.
 * @returns {BigInt} Computed mosaic id.
 */
const generateMosaicId = (ownerAddress, nonce) => {
	const hasher = sha3_256.create();
	hasher.update(uint32ToBytes(nonce));
	hasher.update(ownerAddress.bytes);
	const digest = new Uint8Array(hasher.arrayBuffer());

	let result = digestToBigInt(digest);
	if (result & NAMESPACE_FLAG)
		result -= NAMESPACE_FLAG;

	return result;
};

/**
 * Generates a namespace id from a name and an optional parent namespace id.
 * @param {string} name Namespace name.
 * @param {BigInt} parentNamespaceId Parent namespace id.
 * @returns {BigInt} Computed namespace id.
 */
const generateNamespaceId = (name, parentNamespaceId = BigInt(0)) => {
	const hasher = sha3_256.create();
	hasher.update(uint32ToBytes(Number(parentNamespaceId & BigInt(0xFFFFFFFF))));
	hasher.update(uint32ToBytes(Number((parentNamespaceId >> BigInt(32)) & BigInt(0xFFFFFFFF))));
	hasher.update(name);
	const digest = new Uint8Array(hasher.digest());

	const result = digestToBigInt(digest);
	return result | NAMESPACE_FLAG;
};

/**
 * Returns true if a name is a valid namespace name.
 * @param {string} name Namespace name to check.
 * @returns {boolean} true if the specified name is valid.
 */
const isValidNamespaceName = name => {
	const isAlphanum = character => ('a' <= character && 'z' >= character) || ('0' <= character && '9' >= character);
	if (!name || !isAlphanum(name[0]))
		return false;

	for (let i = 0; i < name.length; ++i) {
		const ch = name[i];
		if (!isAlphanum(ch) && '_' !== ch && '-' !== ch)
			return false;
	}

	return true;
};

/**
 * Parses a fully qualified namespace name into a path.
 * @param {string} fullyQualifiedName Fully qualified namespace name.
 * @returns {array<BigInt>} Computed namespace path.
 */
const generateNamespacePath = fullyQualifiedName => {
	const path = [];
	let parentNamespaceId = BigInt(0);
	fullyQualifiedName.split('.').forEach(name => {
		if (!isValidNamespaceName(name))
			throw Error(`fully qualified name is invalid due to invalid part name (${fullyQualifiedName})`);

		path.push(generateNamespaceId(name, parentNamespaceId));
		parentNamespaceId = path[path.length - 1];
	});

	return path;
};

/**
 * Generates a mosaic id from a fully qualified mosaic alias name.
 * @param {string} fullyQualifiedName Fully qualified mosaic name.
 * @returns {BigInt} Computed mosaic id.
 */
const generateMosaicAliasId = fullyQualifiedName => {
	const path = generateNamespacePath(fullyQualifiedName);
	return path[path.length - 1];
};

module.exports = {
	generateMosaicId, generateNamespaceId, isValidNamespaceName, generateNamespacePath, generateMosaicAliasId
};
