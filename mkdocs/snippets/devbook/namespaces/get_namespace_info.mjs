import { Address, generateNamespacePath } from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL
|| 'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const NAMESPACE_NAME = process.env.NAMESPACE_NAME || 'symbol.xym';
console.log('Namespace name:', NAMESPACE_NAME);

try {
	// Generate namespace ID from name [>step-1]
	const path = generateNamespacePath(NAMESPACE_NAME);
	const namespaceId = path[path.length - 1];
	const namespaceIdHex = namespaceId.toString(16);
	console.log('Namespace ID:', `${namespaceId} (0x${namespaceIdHex})`);
	// [<step-1]
	// Fetch namespace information [>step-2]
	const namespacePath = `/namespaces/${namespaceIdHex}`;
	console.log('Fetching namespace information from', namespacePath);
	const namespaceResponse = await fetch(`${NODE_URL}${namespacePath}`);
	if (!namespaceResponse.ok)
		throw new Error(`HTTP error! status: ${namespaceResponse.status}`);

	const namespaceJSON = await namespaceResponse.json();
	const ns = namespaceJSON.namespace;
	console.log('Namespace information:');
	console.log('  Registration type:', ns.registrationType);
	const ownerAddress = Address.fromDecodedAddressHexString(
		ns.ownerAddress);
	console.log('  Owner address:', ownerAddress.toString());
	const depth = ns.depth;
	console.log('  Depth:', depth);
	console.log('  Level 0 ID:', ns.level0);
	if (2 <= depth)
		console.log('  Level 1 ID:', ns.level1);
	if (3 === depth && ns.level2)
		console.log('  Level 2 ID:', ns.level2);
	console.log('  Start height:', ns.startHeight);
	const endHeight = BigInt(ns.endHeight);
	console.log('  End height:',
		`${endHeight} (0x${endHeight.toString(16).toUpperCase()})`);
	// [<step-2]
	// Display alias information [>step-3]
	const alias = ns.alias;
	console.log('  Alias type:', alias.type);
	if (1 === alias.type) {
		console.log('  Linked mosaic ID:', alias.mosaicId);
	} else if (2 === alias.type) {
		const linkedAddress = Address.fromDecodedAddressHexString(
			alias.address);
		console.log('  Linked address:', linkedAddress.toString());
	} else {
		console.log('  No alias linked');
	} // [<step-3]
} catch (e) {
	console.error(e.message);
}
