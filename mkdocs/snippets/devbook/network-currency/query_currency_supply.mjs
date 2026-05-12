const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);

const SUPPLY_PATH = '/network/currency/supply';
// [>step-1]
const fmt = v => v.toLocaleString('en-US', { minimumFractionDigits: 6 });

const maxResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/max`);
const maximum = parseFloat((await maxResponse.text()).trim());
console.log(`Maximum supply: ${fmt(maximum)} XYM`);

const totalResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/total`);
const total = parseFloat((await totalResponse.text()).trim());
console.log(`Total supply: ${fmt(total)} XYM`);

const circResponse = await fetch(`${NODE_URL}${SUPPLY_PATH}/circulating`);
const circulating = parseFloat((await circResponse.text()).trim());
console.log(`Circulating supply: ${fmt(circulating)} XYM`); // [<step-1]
// [>step-2]
const nonCirculating = total - circulating;
console.log(`Non-circulating: ${fmt(nonCirculating)} XYM`);

const unminted = maximum - total;
console.log(`Unminted: ${fmt(unminted)} XYM`); // [<step-2]
