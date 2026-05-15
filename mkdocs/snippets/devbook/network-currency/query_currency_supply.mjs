const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);

const SUPPLY_PATH = '/network/currency/supply';
// [>step-1]
const fmt = v => v.toLocaleString('en-US', { minimumFractionDigits: 6 });

const maximumResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/max`);
const maximumSupply = parseFloat((await maximumResponse.text()).trim());
console.log(`Maximum supply: ${fmt(maximumSupply)} XYM`);

const totalResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/total`);
const totalSupply = parseFloat((await totalResponse.text()).trim());
console.log(`Total supply: ${fmt(totalSupply)} XYM`);

const circulatingResponse =
	await fetch(`${NODE_URL}${SUPPLY_PATH}/circulating`);
const circulatingSupply =
	parseFloat((await circulatingResponse.text()).trim());
console.log(`Circulating supply: ${fmt(circulatingSupply)} XYM`); // [<step-1]
// [>step-2]
const nonCirculatingSupply = totalSupply - circulatingSupply;
console.log(`Non-circulating supply: ${fmt(nonCirculatingSupply)} XYM`);

const unmintedSupply = maximumSupply - totalSupply;
console.log(`Unminted supply: ${fmt(unmintedSupply)} XYM`); // [<step-2]
