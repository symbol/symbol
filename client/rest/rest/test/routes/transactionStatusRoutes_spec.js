const catapult = require('catapult-sdk');
const transactionStatusRoutes = require('../../src/routes/transactionStatusRoutes');
const test = require('./utils/routeTestUtils');

const { convert } = catapult.utils;

describe('transaction status routes', () => {
	const hashes = [
		'11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF',
		'ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789'
	];
	const binaryHashes = hashes.map(hash => convert.hexToUint8(hash));
	const errorMessage = 'has an invalid format';

	test.route.document.addGetPostDocumentRouteTests(transactionStatusRoutes.register, {
		routes: { singular: '/transaction/:hash/status', plural: '/transaction/statuses' },
		inputs: {
			valid: { object: { hash: hashes[0] }, parsed: [binaryHashes[0]], printable: hashes[0] },
			validMultiple: { object: { hashes }, parsed: binaryHashes },
			invalid: { object: { hash: '12345' }, error: `hash ${errorMessage}` },
			invalidMultiple: { object: { hashes: [hashes[0], '12345', hashes[1]] }, error: `element in array hashes ${errorMessage}` }
		},
		dbApiName: 'transactionsByHashesFailed',
		type: 'transactionStatus',
		config: { transactionStates: [{ dbPostfix: 'Custom', friendlyName: 'custom' }] },
		extendDb: db => {
			// in case of GET: modify the default db function, which returns scalars, to return an array so Array.map works
			(originalTransactionsByHashesFailed => {
				db.transactionsByHashesFailed = (...args) => originalTransactionsByHashesFailed(...args).then(result => {
					// POST:
					if (Array.isArray(result))
						return result;

					// GET:
					return result ? [result] : [];
				});
			})(db.transactionsByHashesFailed);

			db.transactionsByHashesUnconfirmed = () => Promise.resolve([]);
			db.transactionsByHashesCustom = () => Promise.resolve([]);
			db.transactionsByHashes = () => Promise.resolve([]);
		},
		payloadTemplate: { group: 'failed' }
	});
});
