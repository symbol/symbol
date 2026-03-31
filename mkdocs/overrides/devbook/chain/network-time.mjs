const NODE_URL = process.env.NODE_URL
	|| 'https://whydah.symbolmain.net:3001';
console.log(`Using node ${NODE_URL}`);

try {
	// Fetch Nemesis timestamp
	const propertiesPath = '/network/properties';
	console.log(`Fetching network properties from ${propertiesPath}`);
	const propertiesResponse = await fetch(
		`${NODE_URL}${propertiesPath}`);
	const propertiesJson = await propertiesResponse.json();
	const nemesisStr = propertiesJson.network.epochAdjustment;
	const nemesisSeconds = parseInt(nemesisStr.replace('s', ''), 10);
	const nemesisDatetime = new Date(nemesisSeconds * 1000);

	// Fetch current network timestamp
	const timePath = '/node/time';
	console.log(`Fetching current network time from ${timePath}`);
	const timeResponse = await fetch(`${NODE_URL}${timePath}`);
	const timeJson = await timeResponse.json();
	const networkMs = parseInt(
		timeJson.communicationTimestamps.receiveTimestamp, 10);

	const networkDatetime = new Date(
		nemesisDatetime.getTime() + networkMs);

	console.log(`\nNemesis time (UTC): ${nemesisDatetime.toISOString()}`);
	console.log(`Network time (ms since Nemesis): ${networkMs}`);
	console.log(`Network time (UTC): ${networkDatetime.toISOString()}`);
} catch (error) {
	console.error(`Error: ${error.message}`);
}
