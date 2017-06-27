import { expect } from 'chai';
import transactionRoutes from '../../src/routes/transactionRoutes';
import test from './utils/routeTestUtils';

describe('transaction routes', () => {
	describe('send', () => {
		function prepareExecuteRoute(routeName, params, assertRoute) {
			// Arrange: set up a mock server
			const routes = {};
			const server = test.setup.createMockServer('put', routes);

			// - set up the route params
			const routeContext = {
				sendPayloads: [],
				numNextCalls: 0
			};

			const services = {
				connections: {
					lease: () => Promise.resolve({
						send: payload => {
							routeContext.sendPayloads.push(payload);
							return Promise.resolve();
						}
					})
				}
			};

			const next = () => { ++routeContext.numNextCalls; };
			const req = { params };

			routeContext.responses = [];
			const res = { send: (status, response) => { routeContext.responses.push({ status, response }); } };

			// - register the routes
			transactionRoutes.register(server, undefined, services);

			// Act: get the desired route and call it
			const route = test.setup.findRoute(routes, routeName);
			routeContext.routeInvoker = () => route(req, res, next);
			return assertRoute(routeContext);
		}

		it('succeeds if payload is valid', () =>
			// Act:
			prepareExecuteRoute('/transaction/send', { payload: '123456' }, routeContext =>
				routeContext.routeInvoker().then(() => {
					// Assert: next is called
					expect(routeContext.numNextCalls).to.equal(1);

					// - the send payload is correct
					expect(routeContext.sendPayloads.length).to.equal(1);
					expect(routeContext.sendPayloads[0]).to.deep.equal(Buffer.of(
						0x0B, 0x00, 0x00, 0x00, // size (header)
						0x09, 0x00, 0x00, 0x00, // type (header)
						0x12, 0x34, 0x56 // payload
					));

					// - the response is correct
					expect(routeContext.responses.length).to.equal(1);
					expect(routeContext.responses[0]).to.deep.equal({
						status: 202,
						response: { message: 'transaction(s) were pushed to the network' }
					});
				})));

		it('throws if payload is invalid', () =>
			// Act:
			prepareExecuteRoute('/transaction/send', { payload: '1234S6' }, routeContext => {
				test.assert.invokerThrowsError(routeContext.routeInvoker, {
					statusCode: 409,
					message: 'payload has an invalid format: unrecognized hex char \'S6\''
				});
			}));
	});

	describe('get transaction by id', () => {
		function createTransactionRouteInfo(routeName, dbApiName) {
			return {
				routes: transactionRoutes,
				routeName,
				createDb: (queriedIds, entity) => ({
					[dbApiName]: id => {
						queriedIds.push(id);
						return Promise.resolve(entity);
					}
				})
			};
		}

		const transactionRouteInfo = createTransactionRouteInfo('/transaction/id/:id', 'transactionById');
		const Valid_Id = '11223344556677889900AABB';

		it('returns transaction if it is found in db', () =>
				test.route.document.assertReturnsEntityIfFound(transactionRouteInfo, {
					params: { id: Valid_Id },
					paramsIdentifier: Valid_Id,
					dbEntity: { id: 7 },
					type: 'transactionWithMetadata'
				}));

		it('returns 404 if transaction is not found in db', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(transactionRouteInfo, {
				params: { id: Valid_Id },
				paramsIdentifier: Valid_Id,
				printableParamsIdentifier: Valid_Id,
				dbEntity: undefined
			}));

		it('returns 409 if transaction id is invalid', () => Promise.all(['123', '11223344556677889900AAxx'].map(invalidId =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(transactionRouteInfo, {
				params: { id: invalidId },
				error: 'id has an invalid format: must be 12-byte hex string'
			}))));
	});
});
