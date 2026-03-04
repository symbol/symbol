const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
const WS_URL = NODE_URL.replace('http', 'ws') + '/ws';
console.log(`Using node ${NODE_URL}`);

const websocket = new WebSocket(WS_URL);

// connect to websocket endpoint
const uid = await new Promise((resolve) => {
	websocket.addEventListener('message', (event) => {
		const message = JSON.parse(event.data);
		resolve(message.uid);
	}, { once: true });
});
console.log(`Connected to ${WS_URL} with uid ${uid}`);

// subscribe to block channel
websocket.send(JSON.stringify({ uid, subscribe: 'block' }));
console.log('Subscribed to block channel');

// subscribe to finalizedBlock channel
websocket.send(JSON.stringify({ uid, subscribe: 'finalizedBlock' }));
console.log('Subscribed to finalizedBlock channel');

// handle incoming messages
websocket.addEventListener('message', (event) => {
	const message = JSON.parse(event.data);
	const topic = message.topic;

	if (topic === 'block') {
		const block = message.data.block;
		const blockMeta = message.data.meta;
		console.log(
			'New block:'
			+ ` height=${parseInt(block.height, 10).toLocaleString()}`
			+ ` hash=${blockMeta.hash.substring(0, 16)}...`
		);
	}

	if (topic === 'finalizedBlock') {
		const finalized = message.data;
		console.log(
			'Finalized:'
			+ ` height=${parseInt(finalized.height, 10).toLocaleString()}`
			+ ` hash=${finalized.hash.substring(0, 16)}...`
		);
	}
});

// unsubscribe on exit
process.on('SIGINT', () => {
	websocket.send(JSON.stringify({ uid, unsubscribe: 'block' }));
	websocket.send(
		JSON.stringify({ uid, unsubscribe: 'finalizedBlock' }));
	console.log('Unsubscribed from all channels');
	websocket.close();
	process.exit(0);
});
