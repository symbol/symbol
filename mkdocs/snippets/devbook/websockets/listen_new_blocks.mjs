const NODE_URL = process.env.NODE_URL
	|| 'https://reference.symboltest.net:3001';
const WS_URL = `${NODE_URL.replace('http', 'ws')}/ws`;
console.log(`Using node ${NODE_URL}`);

try {
	const websocket = new WebSocket(WS_URL); // [>step-1]

	// Connect to websocket endpoint
	const uid = await new Promise(resolve => {
		websocket.addEventListener('message', event => {
			const message = JSON.parse(event.data);
			resolve(message.uid);
		}, { once: true });
	});
	console.log(`Connected to ${WS_URL} with uid ${uid}`);
	// [<step-1]
	// Subscribe to block channel [>step-2]
	websocket.send(JSON.stringify({ uid, subscribe: 'block' }));
	console.log('Subscribed to block channel');

	// Subscribe to finalizedBlock channel
	websocket.send(JSON.stringify({ uid, subscribe: 'finalizedBlock' }));
	console.log('Subscribed to finalizedBlock channel');
	// [<step-2]
	// Handle incoming messages [>step-3]
	websocket.addEventListener('message', event => {
		const message = JSON.parse(event.data);
		const topic = message.topic;

		if ('block' === topic) {
			const block = message.data.block;
			const blockMeta = message.data.meta;
			console.log(
				'New block:'
				+ ` height=${parseInt(block.height, 10).toLocaleString()}`
				+ ` hash=${blockMeta.hash.substring(0, 16)}...`
			);
		}

		if ('finalizedBlock' === topic) {
			const finalized = message.data;
			console.log(
				'Finalized:'
				+ ' height='
				+ `${parseInt(finalized.height, 10).toLocaleString()}`
				+ ` hash=${finalized.hash.substring(0, 16)}...`
			);
		}
	});
	// [<step-3]
	// Unsubscribe on exit [>step-4]
	process.on('SIGINT', () => {
		websocket.send(JSON.stringify({ uid, unsubscribe: 'block' }));
		websocket.send(
			JSON.stringify({ uid, unsubscribe: 'finalizedBlock' }));
		console.log('Unsubscribed from all channels');
		websocket.close();
		process.exit(0);
	}); // [<step-4]
} catch (error) {
	console.error(error);
}
