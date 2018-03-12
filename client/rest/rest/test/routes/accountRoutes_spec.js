const { expect } = require('chai');
const catapult = require('catapult-sdk');
const accountRoutes = require('../../src/routes/accountRoutes');
const test = require('./utils/routeTestUtils');

const { address } = catapult.model;
const { convert } = catapult.utils;
const { addresses, publicKeys } = test.sets;

describe('account routes', () => {
	describe('get by account', () => {
		const addGetTests = (key, ids, parsedIds) => {
			const errorMessage = 'has an invalid format';
			test.route.document.addGetPostDocumentRouteTests(accountRoutes.register, {
				routes: { singular: '/account/:accountId', plural: '/account' },
				inputs: {
					valid: { object: { accountId: ids[0] }, parsed: [{ [key]: parsedIds[0] }], printable: ids[0] },
					validMultiple: { object: { accountIds: ids }, parsed: parsedIds.map(parsedId => ({ [key]: parsedId })) },
					invalid: { object: { accountId: '12345' }, error: `accountId ${errorMessage}` },
					invalidMultiple: {
						object: { accountIds: [ids[0], '12345', ids[1]] },
						error: `element in array accountIds ${errorMessage}`
					}
				},
				dbApiName: 'accountsByIds',
				type: 'accountWithMetadata',
				config: { transactionStates: [] }
			});
		};

		describe('by address', () =>
			addGetTests('address', addresses.valid, addresses.valid.map(address.stringToAddress)));

		describe('by publicKey', () =>
			addGetTests('publicKey', publicKeys.valid, publicKeys.valid.map(convert.hexToUint8)));

		it('supports lookups of heterogenous ids', () => {
			// Arrange:
			const keyGroups = [];
			const db = test.setup.createCapturingDb('accountsByIds', keyGroups, [{ value: 'this is nonsense' }]);
			const config = { transactionStates: [] };

			// Act:
			const ids = [addresses.valid[0], publicKeys.valid[0], addresses.valid[1]];
			const parsedIds = [
				{ address: address.stringToAddress(ids[0]) },
				{ publicKey: convert.hexToUint8(ids[1]) },
				{ address: address.stringToAddress(ids[2]) }
			];
			return test.route.executeSingle(accountRoutes.register, '/account', 'post', { accountIds: ids }, db, config, response => {
				// Assert:
				expect(keyGroups).to.deep.equal([parsedIds]);
				expect(response).to.deep.equal({ payload: [{ value: 'this is nonsense' }], type: 'accountWithMetadata' });
			});
		});
	});

	describe('account transfers', () => {
		const addAccountTransactionsTests = (apiPath, dbApiPath) => {
			describe(dbApiPath, () => {
				const pagingTestsFactory = test.setup.createPagingTestsFactory(
					{
						routes: accountRoutes,
						routeName: `/account/:publicKey/${apiPath}`,
						createDb: (queriedIdentifiers, transactions) => ({
							[dbApiPath]: (publicKey, pageId, pageSize) => {
								queriedIdentifiers.push({ publicKey, pageId, pageSize });
								return Promise.resolve(transactions);
							}
						}),
						config: { transactionStates: [{ dbPostfix: 'Partial', routePostfix: '/partial' }] }
					},
					{ publicKey: publicKeys.valid[0] },
					{ publicKey: convert.hexToUint8(publicKeys.valid[0]) },
					'transactionWithMetadata'
				);

				pagingTestsFactory.addDefault();
				pagingTestsFactory.addNonPagingParamFailureTest('publicKey', '12345');
			});
		};

		// default transaction states
		addAccountTransactionsTests('transactions', 'accountTransactionsAll');
		addAccountTransactionsTests('transactions/incoming', 'accountTransactionsIncoming');
		addAccountTransactionsTests('transactions/outgoing', 'accountTransactionsOutgoing');
		addAccountTransactionsTests('transactions/unconfirmed', 'accountTransactionsUnconfirmed');

		// custom transaction states (enabled via custom configuration)
		addAccountTransactionsTests('transactions/partial', 'accountTransactionsPartial');
	});
});
