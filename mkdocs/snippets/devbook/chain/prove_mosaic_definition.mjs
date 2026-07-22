import { Hash256, utils } from 'symbol-sdk';
import {
	deserializePatriciaTreeNodes,
	provePatriciaMerkle
} from 'symbol-sdk/symbol';
import crypto from 'crypto';

const { hexToUint8, intToBytes } = utils;

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const sha3_256 = data =>
	crypto.createHash('sha3-256').update(data).digest();

const concat = (...arrays) => {
	const total = arrays.reduce((n, a) => n + a.length, 0);
	const buf = new Uint8Array(total);
	let off = 0;
	for (const a of arrays) {
		buf.set(a, off);
		off += a.length;
	}
	return buf;
};

try {
	// Fetch the network currency mosaic ID [>step-1]
	const propsRes = await fetch(`${NODE_URL}/network/properties`);
	const props = await propsRes.json();
	const rawId = props.chain.currencyMosaicId;
	const mosaicId = BigInt(rawId.replaceAll('\'', ''));
	const mosaicIdHex = mosaicId.toString(16)
		.toUpperCase().padStart(16, '0');
	console.log('Currency mosaic ID:', mosaicIdHex);

	// Fetch the mosaic properties
	const mosaicPath = `/mosaics/${mosaicIdHex}`;
	console.log('Fetching mosaic from', mosaicPath);
	const mosaicRes = await fetch(`${NODE_URL}${mosaicPath}`);
	const mosaicData = await mosaicRes.json();
	const mosaic = mosaicData.mosaic;
	console.log(JSON.stringify(mosaic, undefined, 2));
	// [<step-1]
	// Serialize and hash the mosaic properties [>step-2]
	const serialized = concat(
		intToBytes(parseInt(mosaic.version, 10), 2),
		intToBytes(BigInt(`0x${mosaic.id}`), 8),
		intToBytes(BigInt(mosaic.supply), 8),
		intToBytes(BigInt(mosaic.startHeight), 8),
		hexToUint8(mosaic.ownerAddress),
		intToBytes(parseInt(mosaic.revision, 10), 4),
		intToBytes(parseInt(mosaic.flags, 10), 1),
		intToBytes(parseInt(mosaic.divisibility, 10), 1),
		intToBytes(BigInt(mosaic.duration), 8));
	const hashedValue = new Hash256(sha3_256(serialized));
	console.log('Hashed value:', hashedValue.toString());

	// Hash the mosaic ID to get the encoded key
	const keyBytes = intToBytes(BigInt(`0x${mosaic.id}`), 8);
	const encodedKey = new Hash256(sha3_256(keyBytes));
	console.log('Encoded key:', encodedKey.toString());
	// [<step-2]
	// Fetch the current network height [>step-3]
	const chainRes = await fetch(`${NODE_URL}/chain/info`);
	const chainInfo = await chainRes.json();
	const height = parseInt(chainInfo.height, 10);
	console.log('Current height:', height);

	// Fetch the block's state hash and roots
	const blockPath = `/blocks/${height}`;
	console.log('Fetching block from', blockPath);
	const blockRes = await fetch(`${NODE_URL}${blockPath}`);
	const blockData = await blockRes.json();
	const stateHash = new Hash256(blockData.block.stateHash);
	const roots = blockData.meta.stateHashSubCacheMerkleRoots
		.map(r => new Hash256(r));
	console.log('State hash:', stateHash.toString());
	// [<step-3]
	// Fetch the patricia tree path [>step-4]
	const treeUrl = `/mosaics/${mosaicIdHex}/merkle`;
	console.log('Fetching tree path from', treeUrl);
	const treeRes = await fetch(
		`${NODE_URL}${treeUrl}`);
	const treeData = await treeRes.json();
	const merklePath =
		deserializePatriciaTreeNodes(hexToUint8(treeData.raw));
	console.log('Tree path:', merklePath.length, 'nodes');
	const keyHex = encodedKey.toString();
	let keyPos = 0;
	merklePath.forEach((node, i) => {
		keyPos += node.path.size;
		const pathStr = node.path.size ?
			`  path: ${node.hexPath}` : '';
		if ('value' in node) {
			console.log(`  [${i}] leaf${pathStr}  value: ${node.value}`);
		} else {
			const nibble = keyHex[keyPos];
			keyPos += 1;
			const active = node.links
				.map((l, j) => (l ? j.toString(16).toUpperCase() : null))
				.filter(x => null !== x);
			console.log(
				`  [${i}] branch${pathStr}` +
				`  links: [${active}]` +
				`  -> follow ${nibble}`);
		}
	});
	// [<step-4]
	// Verify the mosaic state [>step-5]
	const result = provePatriciaMerkle(
		encodedKey, hashedValue, merklePath, stateHash, roots);

	if (0x0001 === result) { // VALID_POSITIVE
		console.log(
			`Mosaic ${mosaicIdHex} state verified at height ${height}`);
	} else {
		throw new Error(
			`Mosaic ${mosaicIdHex} proof failed: ${result}`);
	} // [<step-5]
} catch (e) {
	console.error(e.message);
}
