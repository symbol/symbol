/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const { expect } = require('chai');
const catapult = require('catapult-sdk');
const hippie = require('hippie');
const EventEmitter = require('events');
const WebSocket = require('ws');
const zmq = require('zeromq');
const { createZmqConnectionService } = require('../../src/connection/zmqService');
const MessageChannelBuilder = require('../../src/connection/MessageChannelBuilder');
const formatters = require('../../src/server/formatters');
const bootstrapper = require('../../src/server/bootstrapper');
const errors = require('../../src/server/errors');
const test = require('../testUtils');

const supportedHttpMethods = ['get', 'post', 'put'];

const dummyIds = {
	valid: 'valid',
	replayTag: 'replayTag',
	notFound: 'notFound',
	redirect: 'redirect',
	error: 'error',
	asyncValid: 'asyncValid',
	asyncError: 'asyncError'
};

// region dummy route

// note that custom formatting will strip high part
const createChainInfo = (height, scoreLow, scoreHigh) => ({
	id: 123,
	height: [height, height],
	scoreLow: [scoreLow, scoreLow],
	scoreHigh: [scoreHigh, scoreHigh]
});

const addRestRoutes = server => {
	supportedHttpMethods.forEach(method => {
		server[method]('/dummy/:dummyId', (req, res, next) => {
			const { dummyId } = req.params;

			switch (dummyId) {
			case dummyIds.valid: {
				// respond with a valid chain info
				const chainInfo = createChainInfo(10, 16, 11);
				res.send({ payload: chainInfo, type: 'chainInfo' });
				break;
			}

			case dummyIds.replayTag: {
				// respond with a valid chain info computed from the tag parameter
				const tag = req.params.tag | 0; // query parameters are parsed as strings so convert to int
				const chainInfo = createChainInfo(tag, tag, tag);
				res.send({ payload: chainInfo, type: 'chainInfo' });
				break;
			}

			case dummyIds.notFound:
				res.send(errors.createNotFoundError('foo')); // http errors are mapped properly
				break;

			case dummyIds.redirect:
				res.redirect(`/dummy/${dummyIds.valid}`, next);
				return undefined; // don't call next below because it is called by res.redirect

			case dummyIds.asyncValid:
				return Promise.resolve({ height: [11, 11] })
					.then(chainInfo => {
						res.send({ payload: chainInfo, type: 'chainInfo' });
						next();
					});

			case dummyIds.asyncError:
				return Promise.reject(Error('async badness'));

			default:
				throw Error('badness'); // exceptions are handled properly
			}

			// complete non-async, non-exceptional handling
			next();
			return undefined;
		});
	});
};

// endregion

const servers = [];

const createServer = options => {
	const serverFormatters = formatters.create({
		[(options && options.formatterName) || 'json']: {
			chainInfo: {
				// real formatting is not actually being tested, so just drop high part
				format: chainInfo => {
					const formatUint64 = uint64 => (uint64 ? [uint64[0], 0] : undefined);
					return {
						id: chainInfo.id,
						height: formatUint64(chainInfo.height),
						scoreLow: formatUint64(chainInfo.scoreLow),
						scoreHigh: formatUint64(chainInfo.scoreHigh)
					};
				}
			},
			blockHeaderWithMetadata: {
				// real formatting is not actually being tested, so just format a few properties
				format: blockHeaderWithMetadata => {
					const { block } = blockHeaderWithMetadata;
					return {
						height: block.height,
						signer: catapult.utils.convert.uint8ToHex(block.signer)
					};
				}
			}
		}
	});

	const server = bootstrapper.createServer((options || {}).crossDomainHttpMethods, serverFormatters);
	servers.push(server);
	return server;
};

const createWebSocketServer = () => createServer({ formatterName: 'ws' });

describe('server (bootstrapper)', () => {
	afterEach(() => {
		// close servers used during the previous test
		while (0 < servers.length) {
			const server = servers.pop();
			server.close();
		}
	});

	describe('HTTP', () => {
		const wrapHippieEndHandler = handler => (err, res, body) => {
			if (err)
				throw err;

			handler(res.headers, body);
		};

		const makeJsonHippie = (route, method, options) => {
			const server = createServer(options);
			addRestRoutes(server);

			const mockServer = hippie(server).json()[method](route);
			if ('get' === method) {
				// hippie.form() overrides Content-Type to 'application/x-www-form-urlencoded' and uses a matching serializer
				// since hippie.json() was called before, Accept is still 'application/json' and has a matching parser
				mockServer.form();
			}

			// wrap the server to make sure errors are handled appropriately across all tests
			const hippieAdapter = {
				end: handler => {
					mockServer.end(wrapHippieEndHandler(handler));
					return hippieAdapter;
				}
			};

			// expose allowed methods
			['header', 'send', 'expectStatus', 'expectHeader'].forEach(delegatingMethod => {
				hippieAdapter[delegatingMethod] = (...args) => {
					mockServer[delegatingMethod](...args);
					return hippieAdapter;
				};
			});

			return hippieAdapter;
		};

		const assertPayloadHeaders = (headers, expectedContentLength, options = {}) => {
			const shouldAllowCrossDomain = !!options.allowMethods;
			const shouldHaveContent = undefined !== expectedContentLength;

			const message = `received headers: ${JSON.stringify(headers)}`;
			const numExpectedHeaders =
				2
				+ (options.numAdditionalHeaders | 0)
				+ (shouldAllowCrossDomain ? 3 : 0)
				+ (shouldHaveContent ? 2 : 0);
			expect(Object.keys(headers).length, message).to.equal(numExpectedHeaders);

			// these headers should always be stamped
			expect(headers.connection).to.equal('close');
			expect(headers.date).to.not.equal(undefined);

			// these headers should be stamped when there is a response body
			if (shouldHaveContent) {
				expect(headers['content-length'], message).to.equal(expectedContentLength.toString());
				expect(headers['content-type']).to.equal('application/json');
			}

			// these headers should be stamped when cross domain is allowed
			if (shouldAllowCrossDomain) {
				expect(headers['access-control-allow-origin']).to.equal('*');
				expect(headers['access-control-allow-methods']).to.equal(options.allowMethods);
				expect(headers['access-control-allow-headers']).to.equal('Content-Type');
			}
		};

		const addCommonTestsForHttpMethod = method => {
			const methodOptions = {};

			// region sync route handling

			it('handles success properly', done => {
				makeJsonHippie(`/dummy/${dummyIds.valid}`, method)
					.expectStatus(200)
					.end((headers, body) => {
						// Assert:
						assertPayloadHeaders(headers, 63, methodOptions);
						expect(body).to.deep.equal({
							id: 123, height: [10, 0], scoreLow: [16, 0], scoreHigh: [11, 0]
						});
						done();
					});
			});

			it('can parse query params', done => {
				makeJsonHippie(`/dummy/${dummyIds.replayTag}?tag=25`, method)
					.expectStatus(200)
					.end((headers, body) => {
						// Assert:
						assertPayloadHeaders(headers, 63, methodOptions);
						expect(body).to.deep.equal({
							id: 123, height: [25, 0], scoreLow: [25, 0], scoreHigh: [25, 0]
						});
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
						assertPayloadHeaders(headers, 39, methodOptions);
						expect(body).to.deep.equal({ code: 'Internal', message: 'badness' });
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
						assertPayloadHeaders(headers, 45, methodOptions);
						expect(body).to.deep.equal({ code: 'Internal', message: 'async badness' });
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
						assertPayloadHeaders(headers, 69, methodOptions);
						expect(body).to.deep.equal({ code: 'NotAcceptable', message: 'Server accepts: application/json' });
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
						expect(body).to.deep.equal({
							id: 123, height: [10, 0], scoreLow: [16, 0], scoreHigh: [11, 0]
						});
						done();
					});
			});

			it('adds cross domain headers when in configured cross domain http methods ', done => {
				makeJsonHippie(`/dummy/${dummyIds.valid}`, method, { crossDomainHttpMethods: ['FOO', method.toUpperCase(), 'BAR'] })
					.expectStatus(200)
					.end((headers, body) => {
						// Assert:
						assertPayloadHeaders(headers, 63, { allowMethods: `FOO,${method.toUpperCase()},BAR` });
						expect(body).to.deep.equal({
							id: 123, height: [10, 0], scoreLow: [16, 0], scoreHigh: [11, 0]
						});
						done();
					});
			});

			// endregion
		};

		// region unsupported media type

		const runUnsupportedMediaTypeTest = (server, mediaType, sendBody, done) => {
			server
				.header('Content-Type', mediaType)
				.send(sendBody ? { foo: 'bar' } : '')
				.expectStatus(415)
				.end((headers, body) => {
					// Assert:
					assertPayloadHeaders(headers, 44 + mediaType.length);
					expect(body).to.deep.equal({ code: 'UnsupportedMediaType', message: mediaType });
					done();
				});
		};

		const runUnsupportedMediaTypeTestForMethod = (method, mediaType, sendBody, done) => {
			const server = makeJsonHippie(`/dummy/${dummyIds.valid}`, method);
			runUnsupportedMediaTypeTest(server, mediaType, sendBody, done);
		};

		// endregion

		describe('GET', () => {
			addCommonTestsForHttpMethod('get');

			it('rejects request with body with supported media type', done => {
				runUnsupportedMediaTypeTestForMethod('get', 'application/json', true, done);
			});

			it('rejects request with body with unsupported media type', done => {
				runUnsupportedMediaTypeTestForMethod('get', 'application/octet-stream', true, done);
			});
		});

		const addRejectsUnsupportedMediaTypeTests = method => {
			it('rejects request with unsupported (custom) media type without body', done => {
				runUnsupportedMediaTypeTestForMethod(method, 'text/plain', false, done);
			});

			it('rejects request with unsupported (custom) media type with body', done => {
				runUnsupportedMediaTypeTestForMethod(method, 'text/plain', true, done);
			});

			it('rejects request with unsupported (built-in) media type without body', done => {
				runUnsupportedMediaTypeTestForMethod(method, 'application/x-www-form-urlencoded', false, done);
			});

			it('rejects request with unsupported (built-in) media type with body', done => {
				runUnsupportedMediaTypeTestForMethod(method, 'application/x-www-form-urlencoded', true, done);
			});
		};

		const addBodyParsingTest = method => {
			it('can parse json body', done => {
				makeJsonHippie(`/dummy/${dummyIds.replayTag}`, method)
					.send({ tag: 25 })
					.expectStatus(200)
					.end((headers, body) => {
						// Assert:
						assertPayloadHeaders(headers, 63, {});
						expect(body).to.deep.equal({
							id: 123, height: [25, 0], scoreLow: [25, 0], scoreHigh: [25, 0]
						});
						done();
					});
			});
		};

		describe('PUT', () => {
			const method = 'put';
			addCommonTestsForHttpMethod(method);
			addRejectsUnsupportedMediaTypeTests(method);
			addBodyParsingTest(method);
		});

		describe('POST', () => {
			const method = 'post';
			addCommonTestsForHttpMethod(method);
			addRejectsUnsupportedMediaTypeTests(method);
			addBodyParsingTest(method);
		});

		describe('OPTIONS', () => {
			const makeJsonHippieForOptions = route => {
				const server = createServer({ crossDomainHttpMethods: ['FOO', 'OPTIONS', 'BAR'] });
				const routeHandler = (req, res, next) => {
					res.send(200);
					next();
				};

				server.get('/dummy/:dummyId', routeHandler);
				server.post('/dummy/names', routeHandler);
				server.post('/dummy', routeHandler);

				return hippie(server).url(route).method('OPTIONS')
					.json()
					.form();
			};

			const runBasicOptionsTest = (route, expectedMethod, done) => {
				makeJsonHippieForOptions(route)
					.expectStatus(204)
					.expectHeader('allow', expectedMethod)
					.end(wrapHippieEndHandler((headers, body) => {
						// Assert: there should be no body
						assertPayloadHeaders(headers, undefined, { allowMethods: 'FOO,OPTIONS,BAR', numAdditionalHeaders: 1 });
						expect(body).to.equal(null);
						done();
					}));
			};

			it('supports GET', done => {
				runBasicOptionsTest('/dummy/123', 'GET', done);
			});

			it('supports POST', done => {
				runBasicOptionsTest('/dummy', 'POST', done);
			});

			it('prefers exact matches', done => {
				// notice that /dummy/names could potentially match GET /dummy/:dummyId
				runBasicOptionsTest('/dummy/names', 'POST', done);
			});

			it('handles non existent route properly', done => {
				makeJsonHippieForOptions(`/fake/${dummyIds.valid}`)
					.expectStatus(404)
					.end(wrapHippieEndHandler((headers, body) => {
						// Assert: note that non-existent routes never support cross domain
						assertPayloadHeaders(headers, 66);
						expect(body).to.deep.equal({ code: 'ResourceNotFound', message: '/fake/valid does not exist' });
						done();
					}));
			});

			const runUnsupportedMediaTypeTestForOptions = (mediaType, done) => {
				makeJsonHippieForOptions('/dummy')
					.header('Content-Type', mediaType)
					.send({ foo: 'bar' })
					.expectStatus(415)
					.end(wrapHippieEndHandler((headers, body) => {
						// Assert:
						assertPayloadHeaders(headers, 44 + mediaType.length);
						expect(body).to.deep.equal({ code: 'UnsupportedMediaType', message: mediaType });
						done();
					}));
			};

			it('rejects request with body with supported media type', done => {
				runUnsupportedMediaTypeTestForOptions('application/json', done);
			});

			it('rejects request with body with unsupported media type', done => {
				runUnsupportedMediaTypeTestForOptions('application/octet-stream', done);
			});
		});

		describe('other', () => {
			it('rejects invalid methods', done => {
				makeJsonHippie(`/dummy/${dummyIds.valid}`, 'del')
					.expectStatus(405)
					.expectHeader('allow', 'GET, POST, PUT')
					.end((headers, body) => {
						// Assert:
						assertPayloadHeaders(headers, 61, { numAdditionalHeaders: 1 });
						expect(body).to.deep.equal({ code: 'MethodNotAllowed', message: 'DELETE is not allowed' });
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
	});

	describe('websockets', () => {
		// note: although rest server implementation uses single websocket route ('/ws'),
		// server.ws allows you to register any name and you can register multiple different routes
		// the tests are using custom `/ws/block*` routes

		const ports = { server: 1234, mq: 7902 };
		const delays = { publish: 50 };

		const createBlockBuffer = tag => Buffer.concat([
			Buffer.of(0xC0, 0x00, 0x00, 0x00), // size
			Buffer.from(test.random.bytes(test.constants.sizes.signature)), // signature
			Buffer.from('A4C656B45C02A02DEF64F15DD781DD5AF29698A353F414FAAA9CDB364A09F98F', 'hex'), // signer
			Buffer.of(0x03, 0x00, 0x00, 0x80), // version, type
			Buffer.of(0x97, 0x87, 0x45, 0x0E, tag || 0xE1, 0x6C, 0xB6, 0x62), // height
			Buffer.from(test.random.bytes(8)), // timestamp
			Buffer.from(test.random.bytes(8)), // difficulty
			Buffer.from(test.random.bytes(test.constants.sizes.hash256)), // previous block hash
			Buffer.from(test.random.bytes(test.constants.sizes.hash256)) // block transactions hash
		]);

		// notice that the formatter only returns height and signer
		const createFormattedBlock = tag => ({
			height: [0x0E458797, 0x62B66C00 | (tag || 0xE1)],
			signer: 'A4C656B45C02A02DEF64F15DD781DD5AF29698A353F414FAAA9CDB364A09F98F'
		});

		const registerRoute = (server, route) => {
			// create a zmq service that supports only basic (non-transaction) models
			const modelSystem = catapult.plugins.catapultModelSystem.configure([], {});
			const config = {
				host: '127.0.0.1', port: ports.mq, connectTimeout: 1000, monitorInterval: 50
			};
			const channelDescriptors = new MessageChannelBuilder().build();
			const zmqService = createZmqConnectionService(config, modelSystem.codec, channelDescriptors, test.createMockLogger());

			// create a custom emitter for raising client connected events
			const emitter = new EventEmitter();

			// register a ws route (notice that these callbacks make the same calls to zmqService as the callbacks in wsRoutes)
			// except for newClient, which is exclusively used for testing
			server.ws(route, {
				newChannel: (channel, sender) => zmqService.on(channel, object => sender.send(object)),
				removeChannel: channel => zmqService.removeAllListeners(channel),
				newClient: () => { emitter.emit('clientConnected'); }
			});

			return emitter;
		};

		const extractBasicClientOptionValues = options => {
			if ('number' !== typeof options)
				return { numTotalClients: options.numClients, messageIds: options.messageIds };

			// by default, expect all message ids
			const messageIds = new Set();
			for (let i = 1; i <= options; ++i)
				messageIds.add(i);

			return { numTotalClients: options, messageIds };
		};

		const createBoundZsocket = () => {
			const zsocket = zmq.socket('pub');
			zsocket.bindSync(`tcp://127.0.0.1:${ports.mq}`);
			return zsocket;
		};

		const publishBlock = (zsocket, buffer) => {
			// publish the block buffer to the block topic after short delay to allow subscribers to finish attaching
			setTimeout(() => {
				test.log('publishing block data');
				zsocket.send([Buffer.of(0x49, 0x6A, 0xCA, 0x80, 0xE4, 0xD8, 0xF2, 0x9F), buffer]);
			}, delays.publish);
		};

		const createClientSockets = (route, emitter, options, handlers) => {
			const { numTotalClients, messageIds } = extractBasicClientOptionValues(options);

			//  bind to a publisher if one is not provided
			const zsocket = options.zsocket || createBoundZsocket();
			const sockets = [];

			const curryMessageCallback = (ws, id) => messageJson => {
				test.log(`${route} (id ${id}) received message: ${messageJson}`);

				// 1. if uid is sent, subscribe to topic 'block'
				const message = messageJson ? JSON.parse(messageJson) : {};
				if ('uid' in message) {
					const responseJson = JSON.stringify(Object.assign(message, { subscribe: 'block' }));
					ws.send(responseJson);
					test.log('subscribed to block');

					// store the client id in the socket
					ws.uid = message.uid;
					return;
				}

				// 2. if uid is not sent, handle payload (should be block buffer)
				const messageHandler = () => {
					expect(messageIds.has(id), `message id ${id}`).to.equal(true);
					messageIds.delete(id);
					handlers.onMessage(JSON.parse(messageJson));

					if (0 === messageIds.size) {
						test.log('all messages processed');
						handlers.onAllMessages(zsocket, sockets);
					}
				};

				messageHandler(id, messageJson);
			};

			// create web sockets
			let numRemainingClients = numTotalClients;
			for (let i = 1; i <= numTotalClients; ++i) {
				const ws = new WebSocket(`ws://localhost:${ports.server}${route}`);
				sockets.push(ws);
				ws.on('message', curryMessageCallback(ws, i));
			}

			// aggregate test 'clientConnected' events to raise onAllConnected
			emitter.on('clientConnected', () => {
				if (0 === --numRemainingClients) {
					test.log('all clients connected');
					handlers.onAllConnected(zsocket, sockets);
				}
			});
		};

		const createHandlers = (server, done, blockTag = undefined) => ({
			onAllConnected: zsocket => {
				// Act: publish a block
				publishBlock(zsocket, createBlockBuffer(blockTag));
			},
			onMessage: payload => {
				// Assert: notice that payload is already formatted
				expect(payload, `blockTag: ${blockTag}`).to.deep.equal(createFormattedBlock(blockTag));
			},
			onAllMessages: zsocket => {
				// close mq socket and server, otherwise subsequent tests would fail
				zsocket.close();
				server.close();
				done();
			}
		});

		const runSingleRouteTest = (numClients, done) => {
			// Arrange: set up the server with a single ws route
			const server = createWebSocketServer();
			const emitter = registerRoute(server, '/ws/block');
			server.listen(ports.server);

			// Act + Assert: create a client websocket and run the test
			createClientSockets('/ws/block', emitter, numClients, createHandlers(server, done));
		};

		// region subscribe

		it('handles single subscription', done => runSingleRouteTest(1, done));
		it('handles multiple subscriptions to same route', done => runSingleRouteTest(3, done));

		it('handles multiple subscriptions to different routes', done => {
			// Arrange: set up the server with two ws routes
			const server = createWebSocketServer();
			const emitter1 = registerRoute(server, '/ws/block1');
			const emitter2 = registerRoute(server, '/ws/block2');
			server.listen(ports.server);

			const counts = {
				numAllConnectedHandlers: 0,
				numAllMessagesHandlers: 0
			};
			const customHandlers = {
				onAllConnected: zsocket => {
					// - push to the mq only when both websockets are connected
					if (2 === ++counts.numAllConnectedHandlers)
						createHandlers(server, done).onAllConnected(zsocket);
				},
				onAllMessages: zsocket => {
					// - close the server only when messages from both websockets are received and processed
					if (2 === ++counts.numAllMessagesHandlers)
						createHandlers(server, done).onAllMessages(zsocket);
				}
			};

			// - bind to a zsocket
			const zsocket = createBoundZsocket();

			// Act + Assert: create two client websockets pointed to different routes
			// (the routes themselves are meaningless and both will get the same data; the single push above pushes to both routes)
			// (the only difference is that the set of connections and ids are per-route, which is why both connections will have id 1)
			const createOptions = () => ({ numClients: 1, messageIds: new Set([1]), zsocket });
			createClientSockets('/ws/block1', emitter1, createOptions(), Object.assign(createHandlers(server, done), customHandlers));
			createClientSockets('/ws/block2', emitter2, createOptions(), Object.assign(createHandlers(server, done), customHandlers));
		});

		// endregion

		// region unsubscribe

		it('handles unsubscription of client from subscribed channel', done => {
			// Arrange: set up the server with a single ws route
			const server = createWebSocketServer();
			const emitter = registerRoute(server, '/ws/block');
			server.listen(ports.server);

			// - create three client websockets
			const defaultHandlers = createHandlers(server, done);
			const defaultOnAllConnected = defaultHandlers.onAllConnected;
			const defaultOnAllMessages = defaultHandlers.onAllMessages;
			createClientSockets(
				'/ws/block',
				emitter,
				{ numClients: 3, messageIds: new Set([1, 3]) }, // messages should only be sent to the first and last sockets
				Object.assign(defaultHandlers, {
					onAllConnected: (zsocket, sockets) => {
						// Act: unsubscribe the second websocket
						test.log('unsubscribing second websocket');
						sockets[1].send(JSON.stringify({ uid: sockets[1].uid, unsubscribe: 'block' }));
						defaultOnAllConnected(zsocket, sockets);
					},
					onAllMessages: (zsocket, sockets) => {
						// Assert: all sockets are still open
						sockets.forEach(socket => {
							expect(socket.readyState).to.equal(WebSocket.OPEN);
						});

						defaultOnAllMessages(zsocket);
					}
				})
			);
		});

		it('handles unsubscription of client from unknown channel', done => {
			// Arrange: set up the server with a single ws route
			const server = createWebSocketServer();
			const emitter = registerRoute(server, '/ws/block');
			server.listen(ports.server);

			// - create three client websockets
			const defaultHandlers = createHandlers(server, done);
			const defaultOnAllConnected = defaultHandlers.onAllConnected;
			createClientSockets(
				'/ws/block',
				emitter,
				3,
				Object.assign(defaultHandlers, {
					onAllConnected: (zsocket, sockets) => {
						// Act: unsubscribe the second websocket from an unknown channel (this should have no effect)
						test.log('unsubscribing second websocket');
						sockets[1].send(JSON.stringify({ uid: sockets[1].uid, unsubscribe: 'chainInfo' }));
						defaultOnAllConnected(zsocket, sockets);
					}
				})
			);
		});

		// endregion

		// region disconnect (client)

		it('handles disconnecting client sockets', done => {
			// Arrange: set up the server with a single ws route
			const server = createWebSocketServer();
			const emitter = registerRoute(server, '/ws/block');
			server.listen(ports.server);

			// - create three client websockets
			const defaultHandlers = createHandlers(server, done);
			const defaultOnAllConnected = defaultHandlers.onAllConnected;
			createClientSockets(
				'/ws/block',
				emitter,
				{ numClients: 3, messageIds: new Set([1, 3]) }, // messages should only be sent to the first and last sockets
				Object.assign(defaultHandlers, {
					onAllConnected: (zsocket, sockets) => {
						// Act: close the second websocket
						test.log('closing second websocket');
						sockets[1].close();
						defaultOnAllConnected(zsocket, sockets);
					}
				})
			);
		});

		// endregion

		// region invalid subscription requests

		const runInvalidClientTest = (done, messageCallback) => {
			// Arrange: set up the server with a single ws route
			const server = createWebSocketServer();
			registerRoute(server, '/ws/block');
			server.listen(ports.server);

			// - connect four clients to the route
			const numConnections = 4;
			let numCloses = 0;
			const addHandlers = (ws, id) => {
				ws.on('message', messageJson => messageCallback(ws, messageJson));
				ws.on('close', () => {
					// Assert: all clients have been closed
					test.log(`client ${id} was closed`);
					if (numConnections === ++numCloses) {
						// close server, otherwise subsequent tests would fail
						server.close();
						done();
					}
				});
			};

			for (let i = 0; i < numConnections; ++i) {
				const ws = new WebSocket(`ws://localhost:${ports.server}/ws/block`);
				addHandlers(ws, i);
			}
		};

		it('invalid data disconnects client', done => {
			runInvalidClientTest(done, ws => {
				// Act: non-json data
				ws.send('hello');
			});
		});

		it('malformed request disconnects client', done => {
			runInvalidClientTest(done, (ws, messageJson) => {
				// Arrange:
				const message = JSON.parse(messageJson);
				Object.assign(message, { subscribe: 7 });

				// Act: subscribe must be a string
				ws.send(JSON.stringify(message));
			});
		});

		it('unsupported topic subscribe request disconnects client', done => {
			runInvalidClientTest(done, (ws, messageJson) => {
				// Act: try to subscribe to an unsupported topic
				const responseJson = JSON.stringify(Object.assign(JSON.parse(messageJson), { subscribe: 'chainInfo' }));
				ws.send(responseJson);
			});
		});

		// endregion

		// region close (server)

		it('closing server closes all clients', done => {
			// Arrange: set up the server with two ws routes
			const server = createWebSocketServer();
			registerRoute(server, '/ws/block1');
			registerRoute(server, '/ws/block2');
			server.listen(ports.server);

			// - connect two clients to each route
			const numConnections = 4;
			let numOpens = 0;
			let numCloses = 0;

			const addHandlers = (ws, id) => {
				ws.on('open', () => {
					// Act: close the server after all connections have been opened
					if (numConnections === ++numOpens)
						// close server, otherwise subsequent tests would fail
						server.close();
				});

				ws.on('close', () => {
					// Assert: all clients have been closed
					test.log(`client ${id} was closed`);
					if (numConnections === ++numCloses)
						done();
				});
			};

			for (let i = 0; i < numConnections; ++i) {
				const routePostfix = (i % 2) + 1;
				const ws = new WebSocket(`ws://localhost:${ports.server}/ws/block${routePostfix}`);
				addHandlers(ws, i);
			}
		});

		// endregion
	});

	// endregion
});
