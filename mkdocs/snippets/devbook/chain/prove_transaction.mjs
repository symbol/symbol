import { Hash256 } from 'symbol-sdk';
import { proveMerkle } from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const TX_HASH = process.env.TRANSACTION_HASH ||
	'99011A8DBC086E0C359E9D8A38FEC6714C33726FCD0C1B5C0F772A82400D808B';
console.log('Transaction hash:', TX_HASH);

try {
	// Fetch the confirmed transaction to get its block height [>step-1]
	const txPath = `/transactions/confirmed/${TX_HASH}`;
	console.log('Fetching transaction from', txPath);
	const txResponse = await fetch(`${NODE_URL}${txPath}`);
	const txData = await txResponse.json();

	console.log(JSON.stringify(txData.meta, undefined, 2));
	const blockHeight = txData.meta.height;
	const merkleComponentHash = new Hash256(
		txData.meta.merkleComponentHash);
	// [<step-1]
	// Fetch the block header to get the transactions hash [>step-2]
	const blockPath = `/blocks/${blockHeight}`;
	console.log('Fetching block from', blockPath);
	const blockResponse = await fetch(`${NODE_URL}${blockPath}`);
	const blockData = await blockResponse.json();

	console.log(JSON.stringify({
		height: blockData.block.height,
		transactionsHash: blockData.block.transactionsHash
	}, undefined, 2));
	const transactionsHash = new Hash256(
		blockData.block.transactionsHash);
	// [<step-2]
	// Fetch the merkle proof path for the transaction [>step-3]
	const merklePath = `/blocks/${blockHeight}` +
		`/transactions/${TX_HASH}/merkle`;
	console.log('Fetching merkle proof:');
	console.log(`  ${merklePath}`);
	const merkleResponse = await fetch(`${NODE_URL}${merklePath}`);
	const merkleData = await merkleResponse.json();

	console.log(JSON.stringify(merkleData, undefined, 2));

	// Convert the API response to the format expected by the SDK
	const merkleProofPath = merkleData.merklePath.map(
		part => ({
			hash: new Hash256(part.hash),
			isLeft: 'left' === part.position
		}));
	console.log('  Merkle path length:', merkleProofPath.length);
	// [<step-3]
	// Verify that the transaction is included in the block [>step-4]
	const isProven = proveMerkle(
		merkleComponentHash, merkleProofPath, transactionsHash);

	if (isProven) {
		console.log(
			`Transaction ${TX_HASH.slice(0, 16)}...` +
			` proven in block ${blockHeight}`);
	} else {
		throw new Error(
			`Transaction ${TX_HASH.slice(0, 16)}...` +
			` NOT proven in block ${blockHeight}`);
	} // [<step-4]
} catch (e) {
	console.error(e.message);
}
