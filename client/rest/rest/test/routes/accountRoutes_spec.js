import catapult from 'catapult-sdk';
import accountRoutes from '../../src/routes/accountRoutes';
import test from './utils/routeTestUtils';

const address = catapult.model.address;
const convert = catapult.utils.convert;

describe('account routes', () => {
	const factory = {
		createAccountRouteInfo: (routeName, dbApiName) => ({
			routes: accountRoutes,
			routeName,
			createDb: (queriedIdentifiers, account) => ({
				[dbApiName]: id => {
					queriedIdentifiers.push(id);
					return Promise.resolve(account);
				}
			})
		}),
		createAccountTransactionsPagingRouteInfo: (routeName, createDb) => ({
			routes: accountRoutes,
			routeName,
			createDb
		})
	};

	function addAccountGetTests(accountRouteInfo, traits) {
		it('returns account if found', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(accountRouteInfo, {
				params: traits.params,
				paramsIdentifier: traits.paramsIdentifier,
				dbEntity: { id: 8 },
				type: 'accountWithMetadata'
			}));

		it('returns 404 if account is not found', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(accountRouteInfo, {
				params: traits.params,
				paramsIdentifier: traits.paramsIdentifier,
				printableParamsIdentifier: traits.printableParamsIdentifier,
				dbEntity: undefined
			}));
	}

	describe('get (encoded)', () => {
		const accountRouteInfo = factory.createAccountRouteInfo('/account/address/:address', 'accountGet');

		const Valid_Address = 'SAAA244WMCB2JXGNQTQHQOS45TGBFF4V2MJBVOUI';
		addAccountGetTests(accountRouteInfo, {
			params: { address: Valid_Address },
			paramsIdentifier: address.stringToAddress(Valid_Address),
			printableParamsIdentifier: Valid_Address
		});

		it('returns 409 if address is invalid', () =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(accountRouteInfo, {
				params: { address: '12345' },
				error: 'address has an invalid format: 12345 does not represent a valid encoded address'
			}));
	});

	describe('get (public key)', () => {
		const accountRouteInfo = factory.createAccountRouteInfo('/account/key/:publicKey', 'accountGetFromPublicKey');

		const Valid_Public_Key = '75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C';
		addAccountGetTests(accountRouteInfo, {
			params: { publicKey: Valid_Public_Key },
			paramsIdentifier: convert.hexToUint8(Valid_Public_Key),
			printableParamsIdentifier: Valid_Public_Key
		});

		it('returns 409 if key is invalid', () =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(accountRouteInfo, {
				params: { publicKey: '12345' },
				error: 'publicKey has an invalid format: hex string has unexpected size \'5\''
			}));
	});

	describe('account transfers', () => {
		function addAccountTransactionsTests(apiPath, dbApiPath) {
			describe(dbApiPath, () => {
				const Valid_Public_Key = '75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C';
				const pagingTestsFactory = test.setup.createPagingTestsFactory(
					factory.createAccountTransactionsPagingRouteInfo(
						`/account/key/:publicKey/${apiPath}`,
						(queriedIdentifiers, transactions) => ({
							[dbApiPath]: (publicKey, pageId, pageSize) => {
								queriedIdentifiers.push({ publicKey, pageId, pageSize });
								return Promise.resolve(transactions);
							}
						})),
					{ publicKey: Valid_Public_Key },
					{ publicKey: convert.hexToUint8(Valid_Public_Key) },
					'transactionWithMetadata');

				test.assert.addPagingTests(pagingTestsFactory);

				pagingTestsFactory.addFailureTest(
					'key is invalid',
					{ publicKey: '12345' },
					'publicKey has an invalid format: hex string has unexpected size \'5\'');
			});
		}

		addAccountTransactionsTests('transactions', 'accountTransactionsAll');
		addAccountTransactionsTests('transactions/incoming', 'accountTransactionsIncoming');
		addAccountTransactionsTests('transactions/outgoing', 'accountTransactionsOutgoing');
		addAccountTransactionsTests('transactions/unconfirmed', 'accountTransactionsUnconfirmed');
	});
});
