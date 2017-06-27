import { expect } from 'chai';
import hippie from 'hippie';
import WebSocket from 'ws';
import formatters from '../../src/server/formatters';
import bootstrapper from '../../src/server/bootstrapper';
import errors from '../../src/server/errors';
import test from '../testUtils';

const supportedHttpMethods = ['get', 'post', 'put'];

const dummyIds = {
	valid: 'valid',
	notFound: 'notFound',
	redirect: 'redirect',
	error: 'error',
	asyncValid: 'asyncValid',
	asyncError: 'asyncError'
};

// region dummy endpoint

function createChainInfo(height, scoreLow, scoreHigh) {
	// note that custom formatting will strip high part
	return {
		id: 123,
		height: [height, height],
		scoreLow: [scoreLow, scoreLow],
		scoreHigh: [scoreHigh, scoreHigh]
	};
}

function addRestEndpoints(server) {
	for (const method of supportedHttpMethods) {
		server[method]('/dummy/:id', (req, res, next) => {
			const id = req.params.id;

			switch (id) {
			case dummyIds.valid: {
				// respond with a valid chain info
				const chainInfo = createChainInfo(10, 16, 11);
				res.send({ payload: chainInfo, type: 'chainInfo' });
				break;
			}

			case dummyIds.notFound:
				res.send(errors.createNotFoundError('foo')); // http errors are mapped properly
				break;

			case dummyIds.redirect:
				res.redirect(`/dummy/${dummyIds.valid}`, next);
				break;

			case dummyIds.asyncValid:
				return Promise.resolve({ height: [11, 11] })
					.then(chainInfo => {
						res.send({ payload: chainInfo, type: 'chainInfo' });
						next();
					});

			case dummyIds.asyncError:
				return Promise.reject('async badness');

			default:
				throw Error('badness'); // exceptions are handled properly
			}

			// complete non-async, non-exceptional handling
			next();
			return undefined;
		});
	}
}

// endregion

function createServer(options) {
	const formatUint64 = uint64 => (uint64 ? [uint64[0], 0] : undefined);
	const serverFormatters = formatters.create({
		chainInfo: {
			format: chainInfo => ({
				id: chainInfo.id,
				height: formatUint64(chainInfo.height),
				scoreLow: formatUint64(chainInfo.scoreLow),
				scoreHigh: formatUint64(chainInfo.scoreHigh)
			})
		}
	});
	return bootstrapper.createServer((options || {}).crossDomainHttpMethods, serverFormatters);
}

describe('server', () => {
	function makeJsonHippie(endpoint, method, options) {
		const server = createServer(options);
		addRestEndpoints(server);

		const mockServer = hippie(server).json()[method](endpoint);

		// wrap the server to make sure errors are handled appropriately across all tests
		const hippieAdapter = {
			end: handler => {
				mockServer.end((err, res, body) => {
					if (err)
						throw err;

					handler(res.headers, body);
				});
				return hippieAdapter;
			}
		};

		// expose allowed methods
		for (const delegatingMethod of ['header', 'send', 'expectStatus', 'expectHeader']) {
			hippieAdapter[delegatingMethod] = (...args) => {
				mockServer[delegatingMethod](...args);
				return hippieAdapter;
			};
		}

		return hippieAdapter;
	}

	function assertPayloadHeaders(headers, expectedContentLength, options = {}) {
		const shouldAllowCrossDomain = !!options.allowMethods;
		expect(Object.keys(headers).length).to.equal(4 + (options.numAdditionalHeaders | 0) + (shouldAllowCrossDomain ? 3 : 0));
		expect(headers['content-length']).to.equal(expectedContentLength.toString());
		expect(headers['content-type']).to.equal('application/json');

		expect(headers.connection).to.equal('close');
		expect(headers.date).to.not.equal(undefined);

		if (shouldAllowCrossDomain) {
			expect(headers['access-control-allow-origin']).to.equal('*');
			expect(headers['access-control-allow-methods']).to.equal(options.allowMethods);
			expect(headers['access-control-allow-headers']).to.equal('Content-Type');
		}
	}

	function addCommonTestsForHttpMethod(method) {
		const methodOptions = {};

		// region sync route handling

		it('handles success properly', done => {
			makeJsonHippie(`/dummy/${dummyIds.valid}`, method)
				.expectStatus(200)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 63, methodOptions);
					expect(body).to.deep.equal({ id: 123, height: [10, 0], scoreLow: [16, 0], scoreHigh: [11, 0] });
					done();
				});
		});

		it('handles not found properly', done => {
			makeJsonHippie(`/dummy/${dummyIds.notFound}`, method)
				.expectStatus(404)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 72, methodOptions);
					expect(body).to.deep.equal({ code: 'ResourceNotFound', message: 'no resource exists with id \'foo\'' });
					done();
				});
		});

		it('handles error properly', done => {
			makeJsonHippie(`/dummy/${dummyIds.error}`, method)
				.expectStatus(500)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 44, methodOptions);
					expect(body).to.deep.equal({ code: 'InternalError', message: 'badness' });
					done();
				});
		});

		// endregion

		// region async route handling

		it('handles async success properly', done => {
			makeJsonHippie(`/dummy/${dummyIds.asyncValid}`, method)
				.expectStatus(200)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 17, methodOptions);
					expect(body).to.deep.equal({ height: [11, 0] });
					done();
				});
		});

		it('handles async error properly', done => {
			makeJsonHippie(`/dummy/${dummyIds.asyncError}`, method)
				.expectStatus(500)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 50, methodOptions);
					expect(body).to.deep.equal({ code: 'InternalError', message: 'async badness' });
					done();
				});
		});

		// endregion

		// region server errors

		it('handles non existent route properly', done => {
			makeJsonHippie(`/fake/${dummyIds.valid}`, method)
				.expectStatus(404)
				.end((headers, body) => {
					// Assert: note that non-existent routes never support cross domain
					assertPayloadHeaders(headers, 66);
					expect(body).to.deep.equal({ code: 'ResourceNotFound', message: '/fake/valid does not exist' });
					done();
				});
		});

		it('rejects request with invalid accept header', done => {
			makeJsonHippie(`/dummy/${dummyIds.valid}`, method)
				.header('Accept', 'text/plain')
				.expectStatus(406)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 74, methodOptions);
					expect(body).to.deep.equal({ code: 'NotAcceptableError', message: 'Server accepts: application/json' });
					done();
				});
		});

		// endregion

		// region cross domain

		it('does not add cross domain headers when not in configured cross domain http methods ', done => {
			makeJsonHippie(`/dummy/${dummyIds.valid}`, method, { crossDomainHttpMethods: ['FOO', 'BAR'] })
				.expectStatus(200)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 63, { allowMethods: undefined });
					expect(body).to.deep.equal({ id: 123, height: [10, 0], scoreLow: [16, 0], scoreHigh: [11, 0] });
					done();
				});
		});

		it('adds cross domain headers when in configured cross domain http methods ', done => {
			makeJsonHippie(`/dummy/${dummyIds.valid}`, method, { crossDomainHttpMethods: ['FOO', method.toUpperCase(), 'BAR'] })
				.expectStatus(200)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 63, { allowMethods: `FOO,${method.toUpperCase()},BAR` });
					expect(body).to.deep.equal({ id: 123, height: [10, 0], scoreLow: [16, 0], scoreHigh: [11, 0] });
					done();
				});
		});

		// endregion
	}

	describe('GET', () => {
		addCommonTestsForHttpMethod('get');
	});

	function addRejectsUnsupportedMediaTypeTest(method) {
		it('rejects request with unsupported media type', done => {
			makeJsonHippie(`/dummy/${dummyIds.valid}`, method)
				.header('Content-Type', 'text/plain')
				.send({ foo: 'bar' })
				.expectStatus(415)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 59);
					expect(body).to.deep.equal({ code: 'UnsupportedMediaTypeError', message: 'text/plain' });
					done();
				});
		});
	}

	describe('PUT', () => {
		addCommonTestsForHttpMethod('put');
		addRejectsUnsupportedMediaTypeTest('put');
	});

	describe('POST', () => {
		addCommonTestsForHttpMethod('post');
		addRejectsUnsupportedMediaTypeTest('post');
	});

	describe('other', () => {
		it('rejects invalid methods', done => {
			makeJsonHippie(`/dummy/${dummyIds.valid}`, 'del')
				.expectStatus(405)
				.expectHeader('allow', 'GET, POST, PUT')
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 66, { numAdditionalHeaders: 1 });
					expect(body).to.deep.equal({ code: 'MethodNotAllowedError', message: 'DELETE is not allowed' });
					done();
				});
		});

		it('follows redirects', done => {
			// Arrange: 'redirect' should redirect to 'valid'
			makeJsonHippie(`/dummy/${dummyIds.redirect}`, 'get')
				.expectStatus(302)
				.expectHeader('location', `/dummy/${dummyIds.valid}`)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 4, { numAdditionalHeaders: 1 });
					expect(body).to.equal(null);
					done();
				});
		});
	});

	describe('websockets', () => {
		const port = 1234;

		function registerRoute(server, route) {
			let serverWs;
			server.ws(route, ws => { serverWs = ws; });
			return serverWs;
		}

		function createClientSockets(route, options, handlers) {
			let numRemainingClients = options.numClients;
			let messageIds = options.messageIds;

			if ('number' === typeof options) {
				numRemainingClients = options;
				messageIds = new Set();
				for (let i = 1; i <= numRemainingClients; ++i)
					messageIds.add(i);
			}

			function openCallback() {
				if (0 === --numRemainingClients)
					handlers.onAllOpen();
			}

			function curryMessageCallback(id) {
				return text => {
					test.log(`${route} (id ${id}) received message: ${text}`);
					expect(messageIds.has(id), `message id ${id}`).to.equal(true);
					messageIds.delete(id);

					handlers.onMessage(JSON.parse(text));
					if (0 === messageIds.size)
						handlers.onAllMessages();
				};
			}

			const sockets = [];
			const numTotalClients = numRemainingClients;
			for (let i = 1; i <= numTotalClients; ++i) {
				const ws = new WebSocket(`ws://localhost:${port}${route}`);
				sockets.push(ws);

				ws.on('open', openCallback);
				ws.on('message', curryMessageCallback(i));
			}

			return sockets;
		}

		function createHandlers(server, serverWs, done, height, scoreLow, scoreHigh) {
			return {
				onAllOpen: () => {
					// Act: send a payload to the websocket after all connections are established
					serverWs.send({ payload: createChainInfo(height, scoreLow, scoreHigh), type: 'chainInfo' });
				},
				onMessage: payload => {
					// Assert:
					expect(payload).to.deep.equal({ id: 123, height: [height, 0], scoreLow: [scoreLow, 0], scoreHigh: [scoreHigh, 0] });
				},
				onAllMessages: () => {
					server.close();
					done();
				}
			};
		}

		it('handles single connection', done => {
			// Arrange: set up the server with a single ws route
			const server = createServer();
			const serverWs = registerRoute(server, '/ws/chainInfo');
			server.listen(port);

			// - create a client websocket
			createClientSockets(
				'/ws/chainInfo',
				1,
				createHandlers(server, serverWs, done, 17, 6, 11));
		});

		it('handles multiple connections to same route', done => {
			// Arrange: set up the server with a single ws route
			const server = createServer();
			const serverWs = registerRoute(server, '/ws/chainInfo');
			server.listen(port);

			// - create three client websockets
			createClientSockets(
				'/ws/chainInfo',
				3,
				createHandlers(server, serverWs, done, 17, 6, 11));
		});

		it('handles multiple connections to different routes', done => {
			// Arrange: set up the server with two ws routes
			const server = createServer();
			const serverWs1 = registerRoute(server, '/ws/chainInfo1');
			const serverWs2 = registerRoute(server, '/ws/chainInfo2');
			server.listen(port);

			// - create two client websockets pointed to different routes
			let numAllMessagesHandlers = 0;
			const customHandlers = {
				onAllMessages: () => {
					if (2 === ++numAllMessagesHandlers) {
						server.close();
						done();
					}
				}
			};

			createClientSockets(
				'/ws/chainInfo1',
				1,
				Object.assign(createHandlers(server, serverWs1, done, 17, 6, 11), customHandlers));
			createClientSockets(
				'/ws/chainInfo2',
				1,
				Object.assign(createHandlers(server, serverWs2, done, 8, 9, 7), customHandlers));
		});

		it('handles disconnecting client sockets', done => {
			// Arrange: set up the server with a single ws route
			const server = createServer();
			const serverWs = registerRoute(server, '/ws/chainInfo');
			server.listen(port);

			// - create three client websockets
			const defaultHandlers = createHandlers(server, serverWs, done, 17, 6, 11);
			const defaultOnAllOpen = defaultHandlers.onAllOpen;
			const sockets = createClientSockets(
				'/ws/chainInfo',
				{ numClients: 3, messageIds: new Set([1, 3]) }, // messages should only be sent to the first and last sockets
				Object.assign(defaultHandlers, {
					onAllOpen: () => {
						// Act: close the second websocket
						sockets[1].close();
						defaultOnAllOpen();
					}
				}));
		});

		it('handles errors sending data to client sockets', done => {
			// Arrange: set up the server with a single ws route
			const route = '/ws/chainInfo';
			const server = createServer();
			const serverWs = registerRoute(server, route);
			server.listen(port);

			// - create a client websocket
			const ws = new WebSocket(`ws://localhost:${port}${route}`);

			ws.on('open', () => {
				// Act: send bad data (raw payload) followed by good data
				serverWs.send({ payload: createChainInfo(1, 2, 3), type: 'raw' });
				serverWs.send({ payload: createChainInfo(4, 5, 6), type: 'chainInfo' });
			});

			ws.on('message', text => {
				// Assert: an empty message was sent (due to an error)
				test.log(`${route} received message: ${text}`);
				expect(text).to.equal('');
			});

			ws.on('close', () => {
				// Assert: the client was closed
				//         (if the client was not closed, the close event would not fire and this test would hang)
				server.close();
				done();
			});
		});

		it('closing server closes all clients', done => {
			// Arrange: set up the server with two ws routes
			const server = createServer();
			registerRoute(server, '/ws/chainInfo1');
			registerRoute(server, '/ws/chainInfo2');
			server.listen(port);

			// - connect two clients to each route
			const numConnections = 4;
			let numOpens = 0;
			let numCloses = 0;

			function addHandlers(ws, id) {
				ws.on('open', () => {
					// Act: close the server after all connections have been opened
					if (numConnections === ++numOpens)
						server.close();
				});

				ws.on('close', () => {
					// Assert: all clients have been closed
					test.log(`client ${id} was closed`);
					if (numConnections === ++numCloses)
						done();
				});
			}

			for (let i = 0; i < numConnections; ++i) {
				const routePostfix = (i % 2) + 1;
				const ws = new WebSocket(`ws://localhost:${port}/ws/chainInfo${routePostfix}`);
				addHandlers(ws, i);
			}
		});
	});

	// endregion
});
