import { models } from 'symbol-sdk/symbol';

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

const MOSAIC_ID = process.env.MOSAIC_ID || '72C0212E67A08BCE';
console.log('Mosaic ID:', MOSAIC_ID);

try {
	// Fetch mosaic information
	const mosaicPath = `/mosaics/${MOSAIC_ID}`;
	console.log('Fetching mosaic information from', mosaicPath);
	const mosaicResponse = await fetch(`${NODE_URL}${mosaicPath}`);
	if (!mosaicResponse.ok) {
		throw new Error(
			`HTTP error! status: ${mosaicResponse.status}`);
	}
	const mosaicJSON = await mosaicResponse.json();
	const mosaic = mosaicJSON.mosaic;
	console.log('Mosaic information:');
	console.log('  Mosaic ID:', mosaic.id);
	console.log('  Supply:', mosaic.supply);
	const divisibility = mosaic.divisibility;
	console.log('  Divisibility:', divisibility);
	const flags = new models.MosaicFlags(mosaic.flags);
	const flagNames = flags.toString()
		.replace(/MosaicFlags\./g, '').toLowerCase();
	console.log(`  Flags: ${mosaic.flags} (${flagNames})`);
	console.log('  Duration:', mosaic.duration);
	console.log('  Start height:', mosaic.startHeight);
	console.log('  Revision:', mosaic.revision);

	// Display formatted supply
	const supply = BigInt(mosaic.supply);
	const divisor = 10n ** BigInt(divisibility);
	const whole = supply / divisor;
	const fractional = supply % divisor;
	const fractionalStr = fractional.toString()
		.padStart(divisibility, '0');
	console.log(`\nSupply in whole units: ${whole}.${fractionalStr}`);

	// Fetch namespace names linked to the mosaic
	console.log(`\nFetching namespace names for mosaic ${MOSAIC_ID}`);
	const namesResponse = await fetch(
		`${NODE_URL}/namespaces/mosaic/names`, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ mosaicIds: [MOSAIC_ID] })
		});
	const namesInfo = await namesResponse.json();
	for (const entry of namesInfo.mosaicNames) {
		if (entry.names.length > 0) {
			console.log('  Namespace aliases:', entry.names.join(', '));
		} else {
			console.log('  No namespace aliases linked');
		}
	}
} catch (e) {
	console.error(e.message);
}
