import { SymbolFacade } from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Hash of a confirmed tx that used a namespace alias
const TX_HASH = process.env.TRANSACTION_HASH ||
	'BA0C65DB752A3BF1B25285540642537ECE8C2CA716577EDF8BF0F8597A85ADC4';
console.log('Transaction hash:', TX_HASH);

try {
	// Retrieve the confirmed transaction
	const txPath = `/transactions/confirmed/${TX_HASH}`;
	console.log('Fetching transaction from', txPath);
	const txResponse = await fetch(`${NODE_URL}${txPath}`);
	const txData = await txResponse.json();

	const blockHeight = txData.meta.height;
	console.log('  Block height:', blockHeight);

	// primaryId is 1-based, meta.index is 0-based
	const txIndex = txData.meta.index;
	const txPrimary = txIndex + 1;
	console.log(
		`  Transaction index: ${txIndex} (primaryId: ${txPrimary})`);

	const recipientHex = txData.transaction.recipientAddress;
	const recipientBytes = Buffer.from(recipientHex, 'hex');
	const isAddressAlias = (recipientBytes[0] & 0x01) === 1;
	console.log('  Recipient:', recipientHex);
	console.log('  Is address alias:', isAddressAlias);

	const aliasedMosaics = new Set();
	const mosaics = txData.transaction.mosaics;
	for (const mosaic of mosaics) {
		const mosaicId = BigInt(`0x${mosaic.id}`);
		const isMosaicAlias =
			(mosaicId >> 63n & 1n) === 1n;
		if (isMosaicAlias) aliasedMosaics.add(mosaic.id);
		console.log(`  Mosaic: ${mosaic.id}`);
		console.log(
			`  Is mosaic alias: ${isMosaicAlias}`);
	}

	// Query address resolution statements
	if (isAddressAlias) {
		const addressPath =
			'/statements/resolutions/address'
			+ `?height=${blockHeight}`;
		console.log('\nFetching address resolutions from',
			addressPath);
		const addressResponse =
			await fetch(`${NODE_URL}${addressPath}`);
		const addressData = await addressResponse.json();

		const addressStatements = addressData.data;
		console.log(`  Found ${addressStatements.length}`
			+ ' resolution statement(s)');

		for (const item of addressStatements) {
			const statement = item.statement;
			if (statement.unresolved !== recipientHex)
				continue;
			let resolved = null;
			for (const entry of
				statement.resolutionEntries) {
				if (entry.source.primaryId <= txPrimary)
					resolved = entry.resolved;
			}
			if (resolved) {
				const bytes = Uint8Array.from(
					Buffer.from(resolved, 'hex'));
				const address =
					new SymbolFacade.Address(bytes);
				console.log('\nAddress resolution:');
				console.log('  Unresolved:', statement.unresolved);
				console.log(`  Resolved:   ${address}`);
			}
		}
	}

	// Query mosaic resolution statements
	if (aliasedMosaics.size) {
		const mosaicPath =
			'/statements/resolutions/mosaic'
			+ `?height=${blockHeight}`;
		console.log('\nFetching mosaic resolutions from',
			mosaicPath);
		const mosaicResponse =
			await fetch(`${NODE_URL}${mosaicPath}`);
		const mosaicData =
			await mosaicResponse.json();

		const mosaicStatements = mosaicData.data;
		console.log(
			`  Found ${mosaicStatements.length}`
			+ ' resolution statement(s)');

		for (const item of mosaicStatements) {
			const statement = item.statement;
			if (!aliasedMosaics.has(statement.unresolved))
				continue;
			let resolved = null;
			for (const entry of
				statement.resolutionEntries) {
				if (entry.source.primaryId <= txPrimary)
					resolved = entry.resolved;
			}
			if (resolved) {
				console.log('\nMosaic resolution:');
				console.log('  Unresolved:', statement.unresolved);
				console.log(`  Resolved:   ${resolved}`);
			}
		}
	}

} catch (e) {
	console.error(e.message);
}
