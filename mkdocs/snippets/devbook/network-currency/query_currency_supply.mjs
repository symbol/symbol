const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);

const SUPPLY_PATH = '/network/currency/supply';

// Convert the returned decimal string (whole XYM) to atomic units.
// A float conversion would lose precision when the value has more
// than 15 significant digits.
const toAtomic = s => {
	const [whole, frac = ''] = s.trim().split('.');
	return (BigInt(whole) * 1_000_000n) + BigInt(frac.padEnd(6, '0'));
};
// Format an atomic amount back as whole XYM with 6 decimals.
const fmt = v =>
	`${(v / 1_000_000n).toLocaleString('en-US')}.` +
	`${(v % 1_000_000n).toString().padStart(6, '0')}`;

try {
	// [>step-1]
	const maximumResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/max`);
	const maximumSupply = toAtomic(await maximumResponse.text());
	console.log(`Maximum supply: ${fmt(maximumSupply)} XYM`);

	const totalResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/total`);
	const totalSupply = toAtomic(await totalResponse.text());
	console.log(`Total supply: ${fmt(totalSupply)} XYM`);

	const circulatingResponse =
		await fetch(`${NODE_URL}${SUPPLY_PATH}/circulating`);
	const circulatingSupply = toAtomic(await circulatingResponse.text());
	console.log(`Circulating supply: ${fmt(circulatingSupply)} XYM`); // [<step-1]
	// [>step-2]
	const nonCirculatingSupply = totalSupply - circulatingSupply;
	console.log(
		`Non-circulating supply: ${fmt(nonCirculatingSupply)} XYM`);

	const unmintedSupply = maximumSupply - totalSupply;
	console.log(`Unminted supply: ${fmt(unmintedSupply)} XYM`); // [<step-2]
} catch (error) {
	console.log(error);
}
