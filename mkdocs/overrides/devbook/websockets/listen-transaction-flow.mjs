const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
const WS_URL = NODE_URL.replace('http', 'ws') + '/ws';
console.log(`Using node ${NODE_URL}`);

const MONITOR_ADDRESS = process.env.MONITOR_ADDRESS
	|| 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I';
console.log(`Monitoring address: ${MONITOR_ADDRESS}`);

const websocket = new WebSocket(WS_URL);

// Connect and receive uid
const uid = await new Promise((resolve) => {
	websocket.addEventListener('message', (event) => {
		const message = JSON.parse(event.data);
		resolve(message.uid);
	}, { once: true });
});
console.log(`Connected to ${WS_URL} with uid ${uid}`);

// Subscribe to transaction channels
const channels = [
	`confirmedAdded/${MONITOR_ADDRESS}`,
	`unconfirmedAdded/${MONITOR_ADDRESS}`,
	`unconfirmedRemoved/${MONITOR_ADDRESS}`,
];
for (const channel of channels) {
	websocket.send(JSON.stringify({ uid, subscribe: channel }));
	const name = channel.split('/')[0];
	console.log(`Subscribed to ${name} channel`);
}

// Handle incoming messages
websocket.addEventListener('message', (event) => {
	const message = JSON.parse(event.data);
	const topic = message.topic;
	const transactionHash = message.data.meta.hash;
	const name = topic.split('/')[0];
	console.log(`${name}: hash=${transactionHash.substring(0, 16)}...`);
});

// Unsubscribe on exit
process.on('SIGINT', () => {
	for (const channel of channels) {
		websocket.send(JSON.stringify({uid, unsubscribe: channel}));
	}
	console.log('Unsubscribed from all channels');
	websocket.close();
	process.exit(0);
});
