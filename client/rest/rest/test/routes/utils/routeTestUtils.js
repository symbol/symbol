/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const test = require('../../testUtils');
const { expect } = require('chai');
const sinon = require('sinon');

const makeTestName = (base, desc) => (desc ? `${base} ${desc}` : base);

class MockServer {
	constructor() {
		this.routes = {};
		this.server = {};
		['get', 'put', 'post'].forEach(method => {
			this.server[method] = (path, handler) => {
				this.routes[path] = this.routes[path] || {};
				this.routes[path][method] = () => handler;
			};
		});

		this.next = sinon.fake();
		this.send = sinon.fake();
		this.redirect = sinon.fake();
		this.status = sinon.fake();
		this.res = {
			send: this.send,
			redirect: this.redirect,
			status: this.status
		};
	}

	resetStats() {
		this.next.resetHistory();
		this.send.resetHistory();
		this.redirect.resetHistory();
	}

	getRoute(path) {
		return this.routes[path];
	}

	callRoute(route, req) {
		return route(req, this.res, this.next);
	}
}

const routeTestUtils = {
	setup: {
		createMockServer: (captureMethod, routes) => {
			const server = {};
			['get', 'put', 'post', 'ws'].forEach(method => {
				server[method] = () => {};
			});

			server[captureMethod] = (path, handler) => {
				routes[path] = handler;
			};
			return server;
		},

		createCapturingMockServer: (captureMethod, routes) => {
			const server = {};
			['get', 'put', 'post', 'ws'].forEach(method => {
				server[method] = () => {};
			});

			server[captureMethod] = route => routes.push(route);
			return server;
		},

		createCapturingDb: (dbApiName, keyGroups, document) => ({
			[dbApiName]: (...keys) => {
				keyGroups.push(...keys);
				return Promise.resolve(document);
			}
		}),

		createCapturingDbWithExtensions: ({ dbApiName, extendDb }, keyGroups, document) => {
			const db = routeTestUtils.setup.createCapturingDb(dbApiName, keyGroups, document);
			if (extendDb)
				extendDb(db);

			return db;
		},

		findRoute: (routes, routeName) => {
			const route = routes[routeName];
			expect(route).to.not.equal(undefined);
			return route;
		},

		createPagingTestsFactory(routeInfo, routeEntityId, dbEntityId, responseType) {
			const makeCommonParams = (db, params) => [
				routeInfo.routes.register,
				routeInfo.routeName,
				routeInfo.routeCaptureMethod,
				Object.assign({}, routeEntityId, params),
				db,
				routeInfo.config
			];

			const factory = {
				addDefault: () => {
					// - success tests
					const validId = '112233445566778899AABBCC';
					factory.addSuccessTest('basic query', {}, { pageId: undefined, pageSize: 0 });
					factory.addSuccessTest('query with pageId', { id: validId }, { pageId: validId, pageSize: 0 });
					factory.addSuccessTest('query with pageSize', { pageSize: '321' }, { pageId: undefined, pageSize: 321 });
					factory.addSuccessTest(
						'query with pageId and pageSize',
						{ id: validId, pageSize: '321' },
						{ pageId: validId, pageSize: 321 }
					);

					// - failure tests
					factory.addFailureUndefinedDbObjectTest({}, { pageId: undefined, pageSize: 0 });

					factory.addFailureTest('invalid pageId', { id: 'alice', pageSize: '321' }, 'id is not a valid object id');
					factory.addFailureTest(
						'invalid pageSize',
						{ id: validId, pageSize: 'alice' },
						'pageSize is not a valid unsigned integer'
					);
				},

				addSuccessTest: (name, params, expectedParams) =>
					it(name, () => {
						// Arrange:
						const keyGroups = [];
						const payload = [{ id: 100 }, { id: 101 }, { id: 102 }];
						const db = routeInfo.createDb(keyGroups, payload);

						// Act:
						return routeTestUtils.route.executeSingle(...makeCommonParams(db, params), response => {
							// Assert:
							expect(keyGroups).to.deep.equal([Object.assign({}, dbEntityId, expectedParams)]);
							expect(response).to.deep.equal({ payload, type: responseType });
						});
					}),

				addFailureUndefinedDbObjectTest: (params, expectedParams) =>
					it('returns 500 if not array', () => {
						// Arrange:
						const keyGroups = [];
						const db = routeInfo.createDb(keyGroups, undefined);
						const paramId = Object.keys(routeEntityId)[0];

						// Act:
						return routeTestUtils.route.executeSingle(...makeCommonParams(db, params), response => {
							// Assert:
							expect(keyGroups).to.deep.equal([Object.assign({}, dbEntityId, expectedParams)]);
							expect(response.statusCode).to.equal(500);
							expect(response.message).to.equal(`error retrieving data for id: '${paramId}'`);
						});
					}),

				addFailureTest: (name, params, error) =>
					it(`returns 409 if ${name}`, () => {
						// Arrange:
						const db = routeInfo.createDb([], [{ id: 100 }, { id: 101 }, { id: 102 }]);

						// Act:
						return routeTestUtils.route.executeThrows(...makeCommonParams(db, params), error, 409);
					}),

				addNonPagingParamFailureTest: (name, value) =>
					factory.addFailureTest(`${name} is invalid`, { [name]: value }, `${name} has an invalid format`)
			};

			return factory;
		}
	},

	assert: {
		invokerThrowsError: (invoker, expectedError) => {
			try {
				invoker();
			} catch (err) {
				expect(err.statusCode).to.equal(expectedError.statusCode);
				expect(err.message).to.contain(expectedError.message);
				return;
			}

			throw Error('no exception was thrown by test');
		},

		assertRoutes: (routes, expectedRoutes) => {
			expect(routes.length).to.equal(expectedRoutes.length);
			expectedRoutes.forEach(route => {
				expect(routes).to.include(route);
			});
		}
	},

	route: {
		prepareExecuteRoute: (registerRoutes, routeName, routeCaptureMethod, params, db, services, assertRoute) => {
			// Arrange: set up a mock server
			const routes = {};
			const server = routeTestUtils.setup.createMockServer(routeCaptureMethod || 'get', routes);

			// - register the routes
			registerRoutes(server, db, services);

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
				redirect: uri => {
					routeContext.redirects.push(uri);
					next();
				}
			};

			// Act: get the desired route and call it
			const route = routeTestUtils.setup.findRoute(routes, routeName);
			routeContext.routeInvoker = () => route(req, res, next);
			return assertRoute(routeContext);
		},

		executeSingle: (registerRoutes, routeName, routeCaptureMethod, params, db, config, assertResponse) =>
			routeTestUtils.route.prepareExecuteRoute(registerRoutes, routeName, routeCaptureMethod, params, db, { config }, routeContext =>
				routeContext.routeInvoker().then(() => {
					expect(routeContext.numNextCalls, 'next should be called once').to.equal(1);
					expect(routeContext.responses.length, 'single response is expected').to.equal(1);
					expect(routeContext.redirects.length, 'no redirects are expected').to.equal(0);
					assertResponse(routeContext.responses[0]);
				})),

		executeThrows: (registerRoutes, routeName, routeCaptureMethod, params, db, config, expectedMessage, expectedStatusCode) =>
			routeTestUtils.route.prepareExecuteRoute(
				registerRoutes,
				routeName,
				routeCaptureMethod,
				params,
				db,
				{ config },
				routeContext => {
					routeTestUtils.assert.invokerThrowsError(routeContext.routeInvoker, {
						statusCode: expectedStatusCode,
						message: expectedMessage
					});
				}
			),

		executeRedirects: (registerRoutes, routeName, routeCaptureMethod, params, config, assertRedirect) => {
			routeTestUtils.route.prepareExecuteRoute(
				registerRoutes,
				routeName,
				routeCaptureMethod,
				params,
				{},
				{ config },
				routeContext => {
				// redirects happen synchronously, so routeInvoker does not return a promise
					routeContext.routeInvoker();
					expect(routeContext.numNextCalls, 'next should be called once').to.equal(1);
					expect(routeContext.responses.length, 'no responses are expected').to.equal(0);
					expect(routeContext.redirects.length, 'single redirect is expected').to.equal(1);
					assertRedirect(routeContext.redirects[0]);
				}
			);
		},

		document: {
			// prepare tests for resources that support only multiple (GET) retrieval
			prepareGetDocumentsRouteTests: (registerRoutes, rd) => {
				const makeCommonParams = (input, db) => {
					const params = [registerRoutes, rd.route, 'get', input.object];
					if (db)
						params.push(db);

					params.push(rd.config);
					return params;
				};

				return {
					addValidInputTest: (input, desc) => {
						it(makeTestName('returns result if document is found in db', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, [{ value: 'this is nonsense' }]);

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal(input.parsed);
								expect(response).to.deep.equal({ payload: [{ value: 'this is nonsense' }], type: rd.type });
							});
						});
					},

					addEmptyArrayTest: input => {
						it('returns empty array if no documents are found', () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, []);

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal(input.parsed);
								expect(response).to.deep.equal({ payload: [], type: rd.type });
							});
						});
					},

					addNotFoundInputTest: (input, desc) => {
						it(`returns 404 if ${desc || 'documents are not found in db'}`, () => {
							// Arrange: return *something* from the db because the 404 should be triggered by something external
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, [{ value: 'this is nonsense' }]);

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal(input.parsed);
								expect(response.statusCode).to.equal(404);
								expect(response.message).to.equal(`no resource exists with id '${input.printable}'`);
							});
						});
					},

					addRedirectTest: (input, desc) => {
						it(makeTestName('redirects if route parameters are not in range', desc), () => {
							// Arrange:
							const keyGroups = [];

							// Act:
							return routeTestUtils.route.executeRedirects(...makeCommonParams(input, undefined), redirect => {
								// Assert:
								expect(keyGroups).to.deep.equal([]);
								expect(redirect).to.deep.equal(input.redirectUri);
							});
						});
					},

					addInvalidKeyTest: (input, desc) => {
						it(makeTestName('returns 409 if key is invalid', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, [{ value: 'this is nonsense' }]);

							// Act:
							return routeTestUtils.route.executeThrows(...makeCommonParams(input, db), input.error, 409);
						});
					}
				};
			},

			// prepare tests for resources that support only singular (GET) retrieval
			// notice that these are a slightly different subset of prepareGetDocumentsRouteTests
			// (refactoring is possible but would lose readability)
			prepareGetDocumentRouteTests: (registerRoutes, rd) => {
				const makeCommonParams = (input, db) => [registerRoutes, rd.route, 'get', input.object, db, rd.config];

				return {
					addValidInputTest: (input, desc) => {
						it(makeTestName('returns result if document is found in db', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, { value: 'this is nonsense' });

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal(input.parsed);
								const payload = Object.assign({ value: 'this is nonsense' }, rd.payloadTemplate);
								expect(response).to.deep.equal({ payload, type: rd.type });
							});
						});
					},

					addNotFoundInputTest: (input, desc) => {
						it(`returns 404 if ${desc || 'documents are not found in db'}`, () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups);

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal(input.parsed);
								expect(response.statusCode).to.equal(404);
								expect(response.message).to.equal(`no resource exists with id '${input.printable}'`);
							});
						});
					},

					addInvalidKeyTest: (input, desc) => {
						it(makeTestName('returns 409 if key is invalid', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, { value: 'this is nonsense' });

							// Act:
							return routeTestUtils.route.executeThrows(...makeCommonParams(input, db), input.error, 409);
						});
					},

					addDefault(inputs) {
						this.addValidInputTest(inputs.valid);
						this.addNotFoundInputTest(inputs.valid);
						this.addInvalidKeyTest(inputs.invalid);
					}
				};
			},

			// prepare tests for resources that support multiple (POST) retrieval
			preparePostDocumentsRouteTests: (registerRoutes, rd) => {
				const makeCommonParams = (input, db) => [
					registerRoutes,
					rd.routes.plural,
					'post',
					input.object,
					db,
					rd.config
				];

				return {
					addValidInputTest: (input, desc) => {
						it(makeTestName('returns documents if found', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, [{ value: 'this is nonsense' }]);

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal([input.parsed]);
								const payload = [Object.assign({ value: 'this is nonsense' }, rd.payloadTemplate)];
								expect(response).to.deep.equal({ payload, type: rd.type });
							});
						});
					},

					addEmptyArrayTest: (input, desc) => {
						it(makeTestName('returns empty array if no documents are found', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, []);

							// Act:
							return routeTestUtils.route.executeSingle(...makeCommonParams(input, db), response => {
								// Assert:
								expect(keyGroups).to.deep.equal([input.parsed]);
								expect(response).to.deep.equal({ payload: [], type: rd.type });
							});
						});
					},

					addInvalidKeyTest: (input, desc) => {
						it(makeTestName('returns 409 if any key is invalid', desc), () => {
							// Arrange:
							const keyGroups = [];
							const db = routeTestUtils.setup.createCapturingDbWithExtensions(rd, keyGroups, { value: 'this is nonsense' });

							// Act:
							return routeTestUtils.route.executeThrows(...makeCommonParams(input, db), input.error, 409);
						});
					}
				};
			},

			// add tests for resources that support only singular (GET) retrieval
			addGetDocumentRouteTests: (registerRoutes, routesDescriptor) => {
				const builder = routeTestUtils.route.document.prepareGetDocumentRouteTests(registerRoutes, routesDescriptor);
				builder.addDefault(routesDescriptor.inputs);
			},

			// add tests for resources that support both singular (GET) and multiple (POST) retrieval
			addGetPostDocumentRouteTests: (registerRoutes, routesDescriptor) => {
				const rd = routesDescriptor;
				describe('GET', () => {
					routeTestUtils.route.document.addGetDocumentRouteTests(registerRoutes, {
						route: rd.routes.singular,
						inputs: {
							// singular param should be passed as array to db function
							valid: {
								object: rd.inputs.valid.object,
								parsed: [rd.inputs.valid.parsed],
								printable: rd.inputs.valid.printable
							},
							invalid: rd.inputs.invalid
						},
						dbApiName: rd.dbApiName,
						extendDb: rd.extendDb,
						payloadTemplate: rd.payloadTemplate,
						type: rd.type,
						config: rd.config
					});
				});

				describe('POST', () => {
					const builder = routeTestUtils.route.document.preparePostDocumentsRouteTests(registerRoutes, routesDescriptor);
					builder.addValidInputTest(routesDescriptor.inputs.validMultiple);
					builder.addEmptyArrayTest(routesDescriptor.inputs.validMultiple);
					builder.addInvalidKeyTest(routesDescriptor.inputs.invalidMultiple);
				});
			}
		},

		packet: {
			// add tests for routes that send packets to api servers
			addPutPacketRouteTests: (registerRoutes, routeDescriptor) => {
				const rd = routeDescriptor;
				const runTestWithConnections = (params, assertRoute) => {
					// Arrange: set up a mock server
					const routes = {};
					const server = routeTestUtils.setup.createMockServer('put', routes);

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
					registerRoutes(server, undefined, services);

					// Act: get the desired route and call it
					const route = routeTestUtils.setup.findRoute(routes, rd.routeName);
					routeContext.routeInvoker = () => route(req, res, next);
					return assertRoute(routeContext);
				};

				it('succeeds if payload is valid', () =>
					// Act:
					runTestWithConnections(rd.inputs.valid.params, routeContext =>
						routeContext.routeInvoker().then(() => {
							// Assert: next is called
							expect(routeContext.numNextCalls).to.equal(1);

							// - the send payload is correct
							expect(routeContext.sendPayloads.length).to.equal(1);
							expect(routeContext.sendPayloads[0]).to.deep.equal(rd.inputs.valid.parsed);

							// - the response is correct
							expect(routeContext.responses.length).to.equal(1);
							expect(routeContext.responses[0]).to.deep.equal({
								status: 202,
								response: { message: `packet ${rd.packetType} was pushed to the network via ${rd.routeName}` }
							});
						})));

				it('throws if payload is invalid', () =>
					// Act:
					runTestWithConnections(rd.inputs.invalid.params, routeContext => {
						routeTestUtils.assert.invokerThrowsError(routeContext.routeInvoker, {
							statusCode: 409,
							message: `${rd.inputs.invalid.error.key} has an invalid format`
						});
					}));
			}
		}
	},

	sets: {
		addresses: {
			valid: ['SAAA244WMCB2JXGNQTQHQOS45TGBFF4V2MJBVOQ', 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA'],
			invalid: 'SAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1'
		},

		publicKeys: {
			valid: [
				'3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB',
				'75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C'
			],
			invalid: '111111111111111111111111111111111111111111111111111111111111111G'
		},

		hashes256: {
			valid: [
				'D9A0641F8444CD789A83B8789371109E2BB0F7322A4A8E790EC72D580DBBB947',
				'0CDB15C0BEE1AB41AAC540C6AEB30F5DAEDB57BAA062E9E7EA8764BED58B6540'
			],
			invalid: [
				'0CDB15C0BEE1AB41AAC540C6AEB30F5DAEDB57BAA062E9E7EA8764BED58B65408', // + 1
				'0CDB15C0BEE1AB41AAC540C6AEB30F5DAEDB57BAA062E9E7EA8764BED58B654', // - 1
				'ZCDB15C0BEE1AB41AAC540C6AEB30F5DAEDB57BAA062E9E7EA8764BED58B6540' // contains z
			]
		},

		hashes512: {
			valid: [
				'91AC9F58DF84081F0DEFEDB151D3BBC94618587A66CDD255654E6CD32981D6A1'
				+ '4FC839B99AD9E3EFB9E4EB623C44E2BC0441FCEB14FFFA0AB04846C59B65C7BC',
				'8F1DD428F30349F26525C4E7293D1B5AEC256785CA0F7F49A7CB79A514B41670'
				+ '266C69014B95EDAC19F896E68CE31F3E3CAE1884CE13DB67072FB8F1B9044058'
			],
			invalid: [
				'8F1DD428F30349F26525C4E7293D1B5AEC256785CA0F7F49A7CB79A514B41670'
				+ '266C69014B95EDAC19F896E68CE31F3E3CAE1884CE13DB67072FB8F1B90440581', // + 1
				'8F1DD428F30349F26525C4E7293D1B5AEC256785CA0F7F49A7CB79A514B41670'
				+ '266C69014B95EDAC19F896E68CE31F3E3CAE1884CE13DB67072FB8F1B904405', // - 1
				'ZF1DD428F30349F26525C4E7293D1B5AEC256785CA0F7F49A7CB79A514B41670'
				+ '266C69014B95EDAC19F896E68CE31F3E3CAE1884CE13DB67072FB8F1B9044058' // contains z
			]
		}
	}
};
Object.assign(routeTestUtils, test);

module.exports = {
	MockServer,
	test: routeTestUtils
};
