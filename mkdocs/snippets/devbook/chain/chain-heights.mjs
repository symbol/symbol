const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);

let prevHeight = null;
let prevFinalizedHeight = null;
let heightChangedAt = null;
let finalizedChangedAt = null;

try {
	while (true) {
		const response = await fetch(`${NODE_URL}/chain/info`); // [>step-1]
		if (!response.ok) {
			throw new Error(`HTTP error! status: ${response.status}`);
		}
		const chainInfo = await response.json();

		const height = parseInt(chainInfo.height, 10);
		const finalized = chainInfo.latestFinalizedBlock;
		const finalizedHeight = parseInt(finalized.height, 10);
		// [<step-1]
		const now = Date.now();
		// [>step-2]
		if (prevHeight !== null && height !== prevHeight) {
			heightChangedAt = now;
		}
		if (prevFinalizedHeight !== null
			&& finalizedHeight !== prevFinalizedHeight) {
			finalizedChangedAt = now;
		}

		const hAgo = heightChangedAt !== null
			? `${Math.floor((now - heightChangedAt) / 1000)}s ago`
			: '-';
		const fAgo = finalizedChangedAt !== null
			? `${Math.floor((now - finalizedChangedAt) / 1000)}s ago`
			: '-'; // [<step-2]
		// [>step-3]
		const h = height.toLocaleString().padStart(10);
		const fh = finalizedHeight.toLocaleString().padStart(10);
		console.log(
			`Height: ${h}  (changed ${hAgo})`
			+ `  |  Finalized: ${fh}`
			+ `  (changed ${fAgo})`
		);

		prevHeight = height;
		prevFinalizedHeight = finalizedHeight;
		await new Promise((resolve) => setTimeout(resolve, 1000)); // [<step-3]
	}
} catch (error) {
	throw error;
}
