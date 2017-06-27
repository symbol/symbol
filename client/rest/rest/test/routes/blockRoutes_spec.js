import { expect } from 'chai';
import EventEmitter from 'events';
import blockRoutes from '../../src/routes/blockRoutes';
import test from './utils/routeTestUtils';

describe('block routes', () => {
	const Chain_Height = 10;

	function createBlockRouteInfo(routeName, dbApiName) {
		return {
			routes: blockRoutes,
			routeName,
			createDb: (queriedHeights, entity) => ({
				chainInfo: () => Promise.resolve({ height: Chain_Height }),
				[dbApiName]: height => {
					queriedHeights.push(height);
					return Promise.resolve(entity);
				}
			})
		};
	}

	describe('block', () => {
		const blockRouteInfo = createBlockRouteInfo('/block/height/:height', 'blockAtHeight');

		function createTraitsForBlockAtHeight(height) {
			return {
				params: { height: height.toString() },
				paramsIdentifier: height,
				printableParamsIdentifier: height.toString(),
				dbEntity: { id: 7 },
				type: 'blockHeaderWithMetadata'
			};
		}

		it('returns block if request height is less than chain height', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(blockRouteInfo, createTraitsForBlockAtHeight(3)));

		it('returns block if request height is equal to chain height', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(blockRouteInfo, createTraitsForBlockAtHeight(Chain_Height)));

		it('returns 404 if request height is greater than chain height', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(blockRouteInfo, createTraitsForBlockAtHeight(Chain_Height + 1)));

		it('returns 404 if block is not found in db', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(blockRouteInfo, {
				params: { height: '0' },
				paramsIdentifier: 0,
				printableParamsIdentifier: '0',
				dbEntity: undefined
			}));

		it('returns 409 if height is not a number', () =>
			// Arrange:
			test.route.document.assertReturnsErrorForInvalidParams(blockRouteInfo, {
				params: { height: '10A' },
				error: 'height has an invalid format: must be non-negative number'
			}));
	});

	describe('blocks from height', () => {
		const blocksFromRouteInfoWithoutDb = {
			routes: blockRoutes,
			routeName: '/blocks/from/:height/group/:grouping'
		};

		const blocksFromRouteInfo = Object.assign({
			createDb: (queriedIdentifiers, entity) => ({
				blocksFrom: (height, grouping) => {
					queriedIdentifiers.push({ height, grouping });
					return Promise.resolve(entity);
				}
			})
		}, blocksFromRouteInfoWithoutDb);

		function createTraitsForBlocksFromHeight(options) {
			return {
				params: { height: options.height, grouping: options.grouping || '25' },
				paramsIdentifier: options.expected,
				dbEntity: [1, 2, 3],
				type: 'blockHeaderWithMetadata'
			};
		}

		function assertSuccess(options) {
			return test.route.document.assertReturnsEntityIfFound(blocksFromRouteInfo, createTraitsForBlocksFromHeight(options));
		}

		function assertFailure(name, options) {
			return test.route.document.assertReturnsErrorForInvalidParams(blocksFromRouteInfo, {
				params: { height: options.height, grouping: options.grouping },
				error: `${name} has an invalid format: must be non-negative number`
			});
		}

		it('returns blocks if request is valid', () =>
			// Assert:
			assertSuccess({ height: '1', expected: { height: 1, grouping: 25 } }));

		for (const property of ['height', 'grouping']) {
			it(`throws an error if ${property} is invalid`, () =>
				Promise.all(['-12345', '50A'].map(invalidValue =>
					// Assert:
					assertFailure(property, Object.assign({ height: '1234', grouping: '25' }, { [property]: invalidValue })))));
		}

		it('returns blocks if grouping is valid', () =>
			// Assert:
			assertSuccess({ height: '1501', grouping: '100', expected: { height: 1501, grouping: 100 } }));

		function addRedirectTestsWithGrouping(grouping, expectedGrouping) {
			function assertRedirect(height, expectedHeight) {
				return test.route.document.assertReturnsRedirect(blocksFromRouteInfoWithoutDb, {
					params: { height, grouping: grouping.toString() },
					redirectUri: `/blocks/from/${expectedHeight}/group/${expectedGrouping}`
				});
			}

			function addRedirectTests(promises, height) {
				if (0 !== grouping) {
					// height passed is aligned and aligned height does not require a redirect
					promises.push(assertSuccess({ height: height.toString(), grouping: grouping.toString(), expected: { height, grouping } }));
				} else {
					// if grouping is 0 there will be redirect even for aligned height
					promises.push(assertRedirect(height.toString(), height));
				}

				// all non-aligned heights should result in redirect to aligned height
				for (const nonAlignedHeight of [height + 1, height + 2, height + grouping - 1])
					promises.push(assertRedirect(nonAlignedHeight.toString(), height));
			}

			it(`redirect request with height 0 and grouping ${grouping} to height 1`, () =>
				assertRedirect('0', '1'));

			it(`redirects request if height is not multiple of ${grouping}`, () => {
				// Assert:
				const promises = [];
				for (const height of [1, 1 + grouping, 1 + (grouping * 7)])
					addRedirectTests(promises, height);

				return Promise.all(promises);
			});

			it('redirect request if grouping is unknown to grouping 25', () =>
				// Assert:
				test.route.document.assertReturnsRedirect(blocksFromRouteInfoWithoutDb, {
					params: { height: '1501', grouping: (grouping + 1).toString() },
					redirectUri: '/blocks/from/1501/group/25'
				}));
		}

		addRedirectTestsWithGrouping(0, 25);
		addRedirectTestsWithGrouping(25, 25);
		addRedirectTestsWithGrouping(100, 100);
	});

	describe('block web socket', () => {
		function setupWebsocketTest(action) {
			// Arrange:
			const routes = [];
			const server = test.setup.createMockServer('ws', routes);
			const entityEmitter = new EventEmitter();
			blockRoutes.register(server, undefined, { entityEmitter });

			const sentPayloads = [];
			const client = {
				send: data => { sentPayloads.push(data); }
			};

			// Act: get the desired route and call it
			const route = test.setup.findRoute(routes, '/ws/block');
			route(client);

			// - forward to action
			action(entityEmitter, sentPayloads);
		}

		it('forwards emitted block', () => {
			// Act:
			setupWebsocketTest((entityEmitter, sentPayloads) => {
				// - raise a block event
				entityEmitter.emit('block', { id: 7 });

				// Assert:
				expect(sentPayloads.length).to.equal(1);
				expect(sentPayloads[0]).to.deep.equal({
					payload: { id: 7 },
					type: 'blockHeaderWithMetadata'
				});
			});
		});

		it('forwards multiple emitted blocks', () => {
			// Act:
			setupWebsocketTest((entityEmitter, sentPayloads) => {
				// - raise multiple block events
				for (const id of [7, 11, 8])
					entityEmitter.emit('block', { id });

				// Assert:
				let i = 0;
				expect(sentPayloads.length).to.equal(3);
				for (const id of [7, 11, 8]) {
					expect(sentPayloads[i], `payload at ${i}`).to.deep.equal({
						payload: { id },
						type: 'blockHeaderWithMetadata'
					});
					++i;
				}
			});
		});

		it('ignores other emitted entities', () => {
			// Act:
			setupWebsocketTest((entityEmitter, sentPayloads) => {
				// - raise a non-block event
				entityEmitter.emit('transaction', { id: 7 });

				// Assert:
				expect(sentPayloads.length).to.equal(0);
			});
		});
	});

	describe('block transactions', () => {
		const blockTransactionsRouteInfo = createBlockRouteInfo('/block/height/:height/transactions', 'transactionsAtHeight');

		function createTraitsForTransactionsAtHeight(height) {
			return {
				params: { height: height.toString() },
				paramsIdentifier: height,
				printableParamsIdentifier: height.toString(),
				dbEntity: [{ id: 7 }, { id: 8 }],
				type: 'transactionWithMetadata'
			};
		}

		it('returns transactions if request height is less than chain height', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(blockTransactionsRouteInfo, createTraitsForTransactionsAtHeight(3)));

		it('returns transactions if request height is equal to chain height', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(blockTransactionsRouteInfo, createTraitsForTransactionsAtHeight(Chain_Height)));

		it('returns 404 if request height is greater than chain height', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(blockTransactionsRouteInfo, createTraitsForTransactionsAtHeight(Chain_Height + 1)));

		it('returns empty transactions if no transactions are present at height no greater than chain height', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(blockTransactionsRouteInfo, {
				params: { height: '3' },
				paramsIdentifier: 3,
				dbEntity: [],
				type: 'transactionWithMetadata'
			}));

		it('returns 409 if height is not a number', () =>
			// Arrange:
			test.route.document.assertReturnsErrorForInvalidParams(blockTransactionsRouteInfo, {
				params: { height: '10A' },
				error: 'height has an invalid format: must be non-negative number'
			}));

		describe('paging', () => {
			const factory = {
				createBlockTransactionsPagingRouteInfo: (routeName, createDb) => ({
					routes: blockRoutes,
					routeName,
					createDb
				})
			};

			const pagingTestsFactory = test.setup.createPagingTestsFactory(
				factory.createBlockTransactionsPagingRouteInfo(
					'/block/height/:height/transactions',
					(queriedIdentifiers, transactions) => ({
						transactionsAtHeight: (height, pageId, pageSize) => {
							queriedIdentifiers.push({ height, pageId, pageSize });
							return Promise.resolve(transactions);
						},
						chainInfo: () => Promise.resolve({ height: Chain_Height })
					})),
				{ height: '3' },
				{ height: 3 },
				'transactionWithMetadata');

			test.assert.addPagingTests(pagingTestsFactory);

			pagingTestsFactory.addFailureTest(
				'height is invalid',
				{ height: '-1' },
				'height has an invalid format: must be non-negative number');
		});
	});
});
