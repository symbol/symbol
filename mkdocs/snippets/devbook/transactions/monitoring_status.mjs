// Configuration
const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);

// Transaction hash to monitor.
const transactionHash = process.env.TRANSACTION_HASH || // [>step-1]
	'2B6D3B5232E06B9D32682F518C765301FCF9716BFA1EEEF9523653406E04C7EA';
// [<step-1]
console.log(`Monitoring transaction: ${transactionHash}`);
// [>step-2]
/**
 * Poll the transaction status endpoint until the transaction is confirmed.
 * @param {string} txHash - The hash of the transaction to monitor
 * @param {number} maxAttempts - Maximum number of polling attempts
 *   for confirmation
 * @param {number} waitSeconds - Seconds to wait between attempts
 * @returns {boolean} True if transaction was confirmed
 */
async function waitForTransactionConfirmation(
	txHash,
	maxAttempts = 60,
	waitSeconds = 2
) {
	const statusPath = `/transactionStatus/${txHash}`;
	console.log('\nWaiting for transaction confirmation');
	console.log(`Polling ${statusPath}`);

	for (let attempt = 1; attempt <= maxAttempts; attempt++) {
		try {
			// Query the transaction status endpoint
			const statusResponse = await fetch(`${NODE_URL}${statusPath}`);

			if (!statusResponse.ok) {
				const status = statusResponse.status;
				const statusText = statusResponse.statusText;
				const error = new Error(`HTTP ${status}: ${statusText}`);
				error.status = statusResponse.status;
				throw error;
			}

			const statusJSON = await statusResponse.json();

			// Parse the response
			const statusGroup = statusJSON.group;
			const statusCode = statusJSON.code;
			const statusHash = statusJSON.hash;
			const statusDeadline = statusJSON.deadline;

			console.log(`  Attempt ${attempt}:`);
			console.log(`    Status: ${statusGroup}`);
			console.log(`    Code: ${statusCode}`);
			console.log(`    Hash: ${statusHash}`);
			console.log(`    Deadline: ${statusDeadline}`);
			// [<step-2]
			// Check if the transaction has been confirmed [>step-3]
			if ('confirmed' === statusGroup) {
				console.log('\nTransaction confirmed!');
				return true;
			}
			// [<step-3]
			// Check if the transaction failed [>step-4]
			if ('failed' === statusGroup) {
				console.log(
					`\nTransaction failed with code: ${statusCode}`
				);
				throw new Error(`Transaction failed: ${statusCode}`);
			} // [<step-4]
		// [>step-5]
		} catch (error) {
			if (404 === error.status) {
				console.log(
					`  Attempt ${attempt}: Transaction status not yet ` +
					'available'
				);
			} else {
				throw error;
			}
		}
		// [<step-5]
		// Wait before next attempt (except on last attempt) [>step-6]
		if (attempt < maxAttempts) {
			await new Promise(resolve => {
				setTimeout(resolve, waitSeconds * 1000);
			});
		} // [<step-6]
	}
	// [>step-7]
	console.log(
		`\nTransaction not confirmed after ${maxAttempts} attempts`
	);
	throw new Error(
		`Transaction ${txHash} not confirmed in time`
	); // [<step-7]
}

// Monitor the transaction until it's confirmed
await waitForTransactionConfirmation(transactionHash);
