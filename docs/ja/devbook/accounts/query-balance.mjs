const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log('Using node', NODE_URL);


/**
 * Fetch account information by address or public key.
 *
 * @param {string} accountIdentifier - Account address or public key
 * @returns {Promise<Object>} The account information
 */
async function getAccountInfo(accountIdentifier) {
	const accountPath = `/accounts/${accountIdentifier}`;
	const accountResponse = await fetch(`${NODE_URL}${accountPath}`);
	if (!accountResponse.ok) {
		if (accountResponse.status === 404) {
			console.error(
				'Address does not exist:', accountResponse.statusText);
		} else if (accountResponse.status === 409) {
			console.error(
				'Address is not properly formatted:',
				accountResponse.statusText);
		}
		else {
			console.error(
				'Unexpected error:', accountResponse.statusText);
		}
		process.exit(1);
	}
	const accountInfo = await accountResponse.json();
	return accountInfo.account;
}

/**
 * Fetch friendly names for a set of mosaics.
 *
 * @param {bigint[]} mosaicIds - Array of mosaic IDs
 * @returns {Promise<Map>} Map of mosaic IDs to their namespace names
 */
async function getMosaicNames(mosaicIds) {
	const mosaicIdsHex = mosaicIds.map(
		id => id.toString(16).toUpperCase().padStart(16, '0')
	);
	const response = await fetch(`${NODE_URL}/namespaces/mosaic/names`, {
		method: 'POST',
		headers: {'Content-Type': 'application/json'},
		body: JSON.stringify({mosaicIds: mosaicIdsHex})
	});
	const namesInfo = await response.json();
	// Build a map from mosaic IDs to their names
	const namesMap = new Map();
	for (const entry of namesInfo.mosaicNames) {
		const mosaicId = BigInt(`0x${entry.mosaicId}`);
		namesMap.set(mosaicId, entry.names);
	}
	return namesMap;
}

/**
 * Fetch information for multiple mosaics in a single request.
 *
 * @param {bigint[]} mosaicIds - Array of mosaic IDs
 * @returns {Promise<Map>} Map of mosaic IDs to their properties
 */
async function getMosaicsInfo(mosaicIds) {
	const mosaicIdsHex = mosaicIds.map(
		id => id.toString(16).toUpperCase().padStart(16, '0')
	);
	const response = await fetch(`${NODE_URL}/mosaics`, {
		method: 'POST',
		headers: {'Content-Type': 'application/json'},
		body: JSON.stringify({mosaicIds: mosaicIdsHex})
	});
	const mosaicsInfo = await response.json();
	// Build a map from mosaic IDs to their properties
	const mosaicsMap = new Map();
	for (const entry of mosaicsInfo) {
		const mosaicId = BigInt(`0x${entry.mosaic.id}`);
		mosaicsMap.set(mosaicId, entry.mosaic);
	}
	return mosaicsMap;
}

/**
 * Format an atomic amount with decimal places.
 *
 * @param {bigint} amount - The atomic amount
 * @param {number} divisibility - Number of decimal places
 * @returns {string} The formatted amount
 */
function formatAmount(amount, divisibility) {
	if (divisibility === 0) {
		return amount.toString();
	}
	const divisor = 10n ** BigInt(divisibility);
	const wholePart = amount / divisor;
	const fractionalPart = amount % divisor;
	const fractionalStr = fractionalPart.toString()
		.padStart(divisibility, '0');
	return `${wholePart}.${fractionalStr}`;
}

// The account address to query
const ADDRESS = process.env.ADDRESS ||
	'TBIL6D6RURP45YQRWV6Q7YVWIIPLQGLZQFHWFEQ';
console.log('Fetching account information from', ADDRESS);

// Get account information
const account = await getAccountInfo(ADDRESS);

// Display balances for all mosaics the account holds
const accountMosaics = account.mosaics;
if (accountMosaics.length === 0) {
	console.log('Account holds no mosaics');
} else {
	console.log(`Account holds ${accountMosaics.length} mosaic(s):`);

	// Fetch mosaic properties and names for all mosaics
	const mosaicIds = accountMosaics.map(m => BigInt(`0x${m.id}`));
	const mosaicNames = await getMosaicNames(mosaicIds);
	const mosaicsInfo = await getMosaicsInfo(mosaicIds);

	for (const mosaicEntry of accountMosaics) {
		const mosaicId = BigInt(`0x${mosaicEntry.id}`);
		const balance = BigInt(mosaicEntry.amount);

		// Get mosaic properties
		const info = mosaicsInfo.get(mosaicId);
		const divisibility = info.divisibility;

		// Format and display the balance
		const formattedBalance = formatAmount(balance, divisibility);
		const mosaicIdHex =
			`0x${mosaicId.toString(16).toUpperCase().padStart(16, '0')}`;

		// Display mosaic ID and names (if available)
		const names = mosaicNames.get(mosaicId) || [];
		if (names.length > 0) {
			console.log(`- Mosaic ${mosaicIdHex} (${names.join(', ')})`);
		} else {
			console.log(`- Mosaic ${mosaicIdHex}`);
		}

		console.log(`  Balance: ${formattedBalance}`);
		console.log(`  Balance (atomic): ${balance.toString()}`);
		console.log(`  Divisibility: ${divisibility}`);
	}
}
