import { expect } from 'chai';
import test from '../../testUtils';

const routeTestUtils = {
	setup: {
		createMockServer: (captureMethod, routes) => {
			const server = {};
			for (const method of ['get', 'put', 'post', 'ws'])
				server[method] = () => {};

			server[captureMethod] = (path, handler) => {
				routes[path] = handler;
			};
			return server;
		},

		createCapturingMockServer: (captureMethod, routes) => {
			const server = {};
			for (const method of ['get', 'put', 'post', 'ws'])
				server[method] = () => {};

			server[captureMethod] = route => routes.push(route);
			return server;
		},

		findRoute: (routes, routeName) => {
			const route = routes[routeName];
			expect(route).to.not.equal(undefined);
			return route;
		},

		createPagingTestsFactory(routeInfo, routeEntityId, dbEntityId, responseType) {
			return {
				addSuccessTest: (name, params, expectedParams) =>
					it(name, () =>
						routeTestUtils.route.document.assertReturnsEntityIfFound(routeInfo, {
							params: Object.assign({}, routeEntityId, params),
							paramsIdentifier: Object.assign({}, dbEntityId, expectedParams),
							dbEntity: [{ id: 100 }, { id: 101 }, { id: 102 }],
							type: responseType
						})),

				addFailureUndefinedDbObjectTest: (params, expectedParams) =>
					it('returns 500 if not array', () => {
						const paramId = Object.keys(routeEntityId)[0];
						return routeTestUtils.route.document.assertReturnsError(
							routeInfo,
							{
								params: Object.assign({}, routeEntityId, params),
								paramsIdentifier: Object.assign({}, dbEntityId, expectedParams),
								dbEntity: undefined
							},
							500,
							`error retrieving data for id: '${paramId}'`);
					}),

				addFailureTest: (name, params, error) => {
					it(`returns 409 if ${name}`, () =>
						routeTestUtils.route.document.assertReturnsErrorForInvalidParams(routeInfo, {
							params: Object.assign({}, routeEntityId, params),
							error
						}));
				}
			};
		}
	},

	assert: {
		invokerThrowsError: (invoker, expectedError) => {
			try {
				invoker();
			} catch (err) {
				expect(err.statusCode).to.equal(expectedError.statusCode);
				expect(err.message).to.equal(expectedError.message);
				return;
			}

			throw Error('no exception was thrown by test');
		},

		addPagingTests: pagingTestsFactory => {
			// - success tests
			pagingTestsFactory.addSuccessTest('basic query', {}, { pageId: undefined, pageSize: 0 });
			pagingTestsFactory.addSuccessTest(
				'query with pageId',
				{ id: '112233445566778899AABBCC' },
				{ pageId: '112233445566778899AABBCC', pageSize: 0 });
			pagingTestsFactory.addSuccessTest('query with pageSize', { pageSize: '321' }, { pageId: undefined, pageSize: 321 });
			pagingTestsFactory.addSuccessTest(
				'query with pageId and pageSize',
				{ id: '112233445566778899AABBCC', pageSize: '321' },
				{ pageId: '112233445566778899AABBCC', pageSize: 321 });

			// - failure tests
			pagingTestsFactory.addFailureUndefinedDbObjectTest({}, { pageId: undefined, pageSize: 0 });

			pagingTestsFactory.addFailureTest('invalid pageId', { id: 'alice', pageSize: '321' }, 'id is not a valid object id');
			pagingTestsFactory.addFailureTest(
				'invalid pageSize',
				{ id: '112233445566778899AABBCC', pageSize: 'alice' },
				'pageSize is not a valid unsigned integer');
		},

		assertRoutes: (routes, expectedRoutes) => {
			expect(routes.length).to.equal(expectedRoutes.length);
			for (const route of expectedRoutes)
				expect(routes).to.include(route);
		}
	},

	route: {
		prepareExecuteRoute: (registerRoutes, routeName, routeCaptureMethod, params, db, assertRoute) => {
			// Arrange: set up a mock server
			const routes = {};
			const server = routeTestUtils.setup.createMockServer(routeCaptureMethod, routes);

			// - register the routes
			registerRoutes(server, db);

			// - set up the route params
			const routeContext = {
				numNextCalls: 0,
				responses: [],
				redirects: []
			};

			const next = () => { ++routeContext.numNextCalls; };
			const req = { params };

			const res = {
				send: response => { routeContext.responses.push(response); },
				redirect: uri => { routeContext.redirects.push(uri); }
			};

			// Act: get the desired route and call it
			const route = routeTestUtils.setup.findRoute(routes, routeName);
			routeContext.routeInvoker = () => route(req, res, next);
			return assertRoute(routeContext);
		},

		executeSingle: (registerRoutes, routeName, routeCaptureMethod, params, db, assertResponse) =>
			routeTestUtils.route.prepareExecuteRoute(registerRoutes, routeName, routeCaptureMethod || 'get', params, db, routeContext =>
				routeContext.routeInvoker().then(() => {
					expect(routeContext.numNextCalls).to.equal(1);
					expect(routeContext.responses.length).to.equal(1);
					expect(routeContext.redirects.length).to.equal(0);
					assertResponse(routeContext.responses[0]);
				})),

		executeThrows: (registerRoutes, routeName, routeCaptureMethod, params, db, expectedMessage, expectedStatusCode) =>
			routeTestUtils.route.prepareExecuteRoute(registerRoutes, routeName, routeCaptureMethod || 'get', params, db, routeContext => {
				routeTestUtils.assert.invokerThrowsError(routeContext.routeInvoker, {
					statusCode: expectedStatusCode,
					message: expectedMessage
				});
			}),

		executeRedirects: (registerRoutes, routeName, params, assertRedirect) => {
			routeTestUtils.route.prepareExecuteRoute(registerRoutes, routeName, 'get', params, {}, routeContext => {
				// redirects happen synchronously, so routeInvoker does not return a promise
				routeContext.routeInvoker();
				expect(routeContext.numNextCalls).to.equal(1);
				expect(routeContext.responses.length).to.equal(0);
				expect(routeContext.redirects.length).to.equal(1);
				assertRedirect(routeContext.redirects[0]);
			});
		},

		document: {
			assertReturnsEntityIfFound: (routeInfo, traits) => {
				// Arrange:
				const queriedIdentifiers = [];
				const db = routeInfo.createDb(queriedIdentifiers, traits.dbEntity);

				// Act:
				return routeTestUtils.route.executeSingle(
					routeInfo.routes.register,
					routeInfo.routeName,
					routeInfo.routeCaptureMethod,
					traits.params,
					db,
					response => {
						// Assert:
						expect(queriedIdentifiers).to.deep.equal([traits.paramsIdentifier]);
						expect(response).to.deep.equal({ payload: traits.dbEntity, type: traits.type });
					});
			},

			assertReturnsError: (routeInfo, traits, errorCode, errorMessage) => {
				// Arrange:
				const queriedIdentifiers = [];
				const db = routeInfo.createDb(queriedIdentifiers, traits.dbEntity);

				// Act:
				return routeTestUtils.route.executeSingle(
					routeInfo.routes.register,
					routeInfo.routeName,
					routeInfo.routeCaptureMethod,
					traits.params,
					db,
					response => {
						// Assert:
						expect(queriedIdentifiers).to.deep.equal([traits.paramsIdentifier]);
						expect(response.statusCode).to.equal(errorCode);
						expect(response.message).to.equal(errorMessage);
					});
			},

			assertReturnsErrorIfNotFound: (routeInfo, traits) =>
				routeTestUtils.route.document.assertReturnsError(
					routeInfo,
					traits,
					404,
					`no resource exists with id '${traits.printableParamsIdentifier}'`),

			assertReturnsRedirect: (routeInfo, traits) => {
				// Arrange:
				const queriedIdentifiers = [];

				// Act:
				return routeTestUtils.route.executeRedirects(routeInfo.routes.register, routeInfo.routeName, traits.params, redirect => {
					// Assert:
					expect(queriedIdentifiers).to.deep.equal([]);
					expect(redirect).to.deep.equal(traits.redirectUri);
				});
			},

			assertReturnsErrorForInvalidParams: (routeInfo, traits) => {
				// Arrange:
				const db = routeInfo.createDb([], traits.dbEntity);

				// Act:
				return routeTestUtils.route.executeThrows(
					routeInfo.routes.register,
					routeInfo.routeName,
					routeInfo.routeCaptureMethod,
					traits.params,
					db,
					traits.error,
					409);
			}
		}
	}
};
Object.assign(routeTestUtils, test);

export default routeTestUtils;
