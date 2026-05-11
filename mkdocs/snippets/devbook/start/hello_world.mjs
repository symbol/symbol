import {
	SymbolFacade,
	NetworkTimestamp
} from 'symbol-sdk/symbol';
// [>step-1]
const facade = new SymbolFacade('mainnet');
console.log(`Network name: ${facade.network.name}`);
// NetworkTimestamp(0) is the genesis block timestamp (network launch)
const launchDate = facade.network.toDatetime(new NetworkTimestamp(0));
console.log(`Network launch date: ${launchDate.toISOString()}`); // [<step-1]
// [>step-2]
const NODE_URL = 'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);
try {
	// Fetch current chain information
	const infoPath = '/chain/info';
	console.log(`Fetching chain information from ${infoPath}`);
	const response = await fetch(`${NODE_URL}${infoPath}`,
		{ timeout: 10000 });
	if (!response.ok) {
		throw new Error(`HTTP error! status: ${response.status}`);
	}
	const responseJson = await response.json();
	const height = parseInt(responseJson.height, 10);
	console.log(`  Blockchain height: ${height.toLocaleString()} blocks`);
} catch (e) {
	console.error(e.message, '| Cause:', e.cause?.code ?? 'unknown');
} // [<step-2]
