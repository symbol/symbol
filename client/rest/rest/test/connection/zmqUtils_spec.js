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

const zmqUtils = require('../../src/connection/zmqUtils');
const test = require('../testUtils');
const { expect } = require('chai');

describe('zmqUtils', () => {
	const createMockZsocket = () => {
		const zsocket = {
			monitorParamGroups: [],
			numUnmonitorCalls: 0,
			numCloseCalls: 0,
			emittedEventNames: [],
			eventHandlers: {}
		};

		zsocket.monitor = (interval, numEvents) => { zsocket.monitorParamGroups.push({ interval, numEvents }); };
		zsocket.unmonitor = () => { ++zsocket.numUnmonitorCalls; };
		zsocket.close = () => { ++zsocket.numCloseCalls; };

		zsocket.emit = eventName => { zsocket.emittedEventNames.push(eventName); };

		// treat on and once similarly
		zsocket.on = (eventName, eventHandler) => {
			if (!(eventName in zsocket.eventHandlers))
				zsocket.eventHandlers[eventName] = [];

			zsocket.eventHandlers[eventName].push(eventHandler);
		};
		zsocket.once = zsocket.on;
		return zsocket;
	};

	describe('prepareZsocket', () => {
		const zmqConfig = { connectTimeout: 10000, monitorInterval: 150 };

		const prepareDefaultZsocket = zsocket => {
			zmqUtils.prepareZsocket(zsocket, zmqConfig, test.createMockLogger());
		};

		const cancelConnectTimer = zsocket => {
			zsocket.eventHandlers.connect[1](); // cancel the timer
		};

		const assertZsocketIsOpen = zsocket => {
			expect(zsocket.monitorParamGroups).to.deep.equal([{ interval: 150, numEvents: 0 }]);
			expect(zsocket.numUnmonitorCalls).to.equal(0);
			expect(zsocket.numCloseCalls).to.equal(0);
			expect(zsocket.emittedEventNames).to.deep.equal([]);
		};

		const assertZsocketIsClosed = zsocket => {
			expect(zsocket.numUnmonitorCalls).to.equal(1);
			expect(zsocket.numCloseCalls).to.equal(1);
			expect(zsocket.emittedEventNames).to.deep.equal(['zsocket_close']);
		};

		it('enables socket event monitoring', () => {
			// Arrange:
			const zsocket = createMockZsocket();

			// Act:
			prepareDefaultZsocket(zsocket);
			cancelConnectTimer(zsocket);

			// Assert:
			assertZsocketIsOpen(zsocket);
		});

		describe('monitor event logging handlers ', () => {
			const runMonitorLoggingTest = (options, createLogPromise) => {
				// Arrange:
				const zsocket = createMockZsocket();
				const logger = test.createMockLogger();
				const monitorEventNames = [
					'connect', 'connect_delay', 'connect_retry',
					'listen', 'bind_error',
					'accept', 'accept_error',
					'close', 'close_error',
					'disconnect',
					'monitor_error'
				];

				// Act:
				zmqUtils.prepareZsocket(zsocket, Object.assign({ monitorLoggingThrottle: options.throttle }, zmqConfig), logger);
				cancelConnectTimer(zsocket);

				// - all events have a (presumably logging) handler registered
				const promises = [];
				monitorEventNames.forEach((eventName, index) => {
					const handler = zsocket.eventHandlers[eventName][0];
					expect(handler, eventName).to.be.a('function');

					// - invoke the handler
					promises.push(createLogPromise(handler));

					// Assert: the expected number of logs were made synchronously
					expect(logger.numLogs, `${eventName} at ${index}`).to.equal((index + 1) * options.numLogsPerInvokeSync);
				});

				return Promise.all(promises).then(() => {
					// Assert: the expected number of total logs were made
					expect(logger.numLogs).to.equal(monitorEventNames.length * options.numLogsPerInvokeTotal);

					// - there are no side-effects
					assertZsocketIsOpen(zsocket);
				});
			};

			it('are registered for all zmq events', () =>
				// Arrange:
				runMonitorLoggingTest({ throttle: 0, numLogsPerInvokeSync: 1, numLogsPerInvokeTotal: 1 }, handler => {
					// Act: invoke the handler immediately
					handler();
					return Promise.resolve();
				}));

			it('throttle noisy logs', () =>
				// Arrange: successive logs for same zsocket and event should be throttled
				runMonitorLoggingTest({ throttle: 50, numLogsPerInvokeSync: 1, numLogsPerInvokeTotal: 1 }, handler => {
					// Act: invoke the handler immediately multiple times
					handler();
					handler();
					handler();
					return Promise.resolve();
				}));

			it('allow additional logs outside of throttle period', () =>
				// Arrange: delayed logs for same zsocket and event should be allowed
				runMonitorLoggingTest({ throttle: 50, numLogsPerInvokeSync: 1, numLogsPerInvokeTotal: 2 }, handler =>
					// Act: invoke the handler immediately
					new Promise(resolve => {
						handler();
						handler();
						handler();

						// - invoke the handler after the throttle period
						setTimeout(() => {
							handler();
							handler();
							handler();
							resolve();
						}, 100);
					})));
		});

		it('customizes close behavior', () => {
			// Arrange:
			const zsocket = createMockZsocket();

			// - override emit in order to be able to verify ordering
			const emitCaptures = [];
			const originalEmit = zsocket.emit;
			zsocket.emit = eventName => {
				emitCaptures.push({ eventName, numCloseCalls: zsocket.numCloseCalls });
				originalEmit.call(zsocket, eventName);
			};

			prepareDefaultZsocket(zsocket);
			cancelConnectTimer(zsocket);

			// Act:
			zsocket.close();

			// Assert:
			assertZsocketIsClosed(zsocket);

			// - close event was raised after socket was closed
			expect(emitCaptures).to.deep.equal([{ eventName: 'zsocket_close', numCloseCalls: 1 }]);
		});

		it('closes socket on error', () => {
			// Arrange:
			const zsocket = createMockZsocket();
			prepareDefaultZsocket(zsocket);
			cancelConnectTimer(zsocket);

			// Act:
			zsocket.eventHandlers.error[0](new Error());

			// Assert:
			assertZsocketIsClosed(zsocket);
		});

		it('closes socket on connect timeout', () => {
			// Arrange:
			const zsocket = createMockZsocket();
			zmqUtils.prepareZsocket(zsocket, { connectTimeout: 0, monitorInterval: 150 }, test.createMockLogger());

			// Act: wait for the timeout
			return new Promise(resolve => {
				setTimeout(() => {
					// Assert:
					assertZsocketIsClosed(zsocket);
					resolve();
				}, 1);
			});
		});

		it('only calls close once when explicit close is followed by timeout', () => {
			// Arrange:
			const zsocket = createMockZsocket();
			zmqUtils.prepareZsocket(zsocket, { connectTimeout: 0, monitorInterval: 150 }, test.createMockLogger());

			// Act: explicitly close the socket
			zsocket.close();

			// - let the timer fire and attempt to close the socket again
			return new Promise(resolve => {
				setTimeout(() => {
					// Assert: close was only called once on the underlying socket
					assertZsocketIsClosed(zsocket);
					resolve();
				}, 1);
			});
		});

		it('clears connect timer on successful connect', () => {
			// Arrange:
			const zsocket = createMockZsocket();
			zmqUtils.prepareZsocket(zsocket, { connectTimeout: 0, monitorInterval: 150 }, test.createMockLogger());

			// Act: simulate a connect
			cancelConnectTimer(zsocket);

			// - give the timer a chance to fire (it will not because it is cancelled)
			return new Promise(resolve => {
				setTimeout(() => {
					// Assert: close was not called because the timer was cancelled
					assertZsocketIsOpen(zsocket);
					resolve();
				}, 1);
			});
		});
	});

	describe('createMultisocketEmitter', () => {
		const createMockZsocketWithCapture = context => {
			context.zsockets = {};
			return (key, eventEmitter) => {
				context.eventEmitter = eventEmitter;
				context.zsockets[key] = createMockZsocket();
				return context.zsockets[key];
			};
		};

		it('initially has no sockets', () => {
			// Arrange:
			const emitter = zmqUtils.createMultisocketEmitter(() => {});

			// Act + Assert:
			expect(emitter.zsocketCount()).to.equal(0);
		});

		describe('on', () => {
			it('creates socket when new channel is subscribed', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);

				// Act:
				emitter.on('block', () => {});

				// Assert:
				expect(emitter.zsocketCount()).to.equal(1);
				expect(emitter.listenerCount('block')).to.equal(1);

				// Sanity: no other listeners are set up
				expect(emitter.listenerCount('block.close')).to.equal(0);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(0);
			});

			it('creates socket per channel', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);

				// Act:
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('partialAdded', () => {});

				// Assert:
				expect(emitter.zsocketCount()).to.equal(3);
				expect(emitter.listenerCount('block')).to.equal(1);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(1);
				expect(emitter.listenerCount('partialAdded')).to.equal(1);
			});

			it('reuses socket when existing channel is subscribed', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);

				// Act:
				emitter.on('block', () => {});
				emitter.on('block', () => {});
				emitter.on('block', () => {});

				// Assert:
				expect(emitter.zsocketCount()).to.equal(1);
				expect(emitter.listenerCount('block')).to.equal(3);
			});

			it('bypasses socket creation when supported subevent is subscribed', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);

				// Act:
				emitter.on('block.close', () => {});

				// Assert:
				expect(emitter.zsocketCount()).to.equal(0);
				expect(emitter.listenerCount('block.close')).to.equal(1);
			});

			it('fails when unsupported subevent is subscribed', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);

				// Act + Assert:
				expect(() => emitter.on('block.foo', () => {}).to.throw('block.foo indicates an unsupported subevent'));
			});

			const assertOnlySubscribedHandlersAreInvoked = (eventName1, eventName2, numExpectedSockets) => {
				// Arrange:
				const context = {};
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocketWithCapture(context));
				const captures = {};

				// - set up five subscriptions to two different channels
				emitter.on(eventName1, value => { captures.a = value; });
				emitter.on(eventName2, value => { captures.b = value; });
				emitter.on(eventName1, value => { captures.c = value; });
				emitter.on(eventName2, value => { captures.d = value; });
				emitter.on(eventName1, value => { captures.e = value; });

				// Act: emit events
				context.eventEmitter.emit(eventName1, 1);
				context.eventEmitter.emit(eventName2, 2);

				// Assert:
				expect(emitter.zsocketCount()).to.equal(numExpectedSockets);
				expect(emitter.listenerCount(eventName1)).to.equal(3);
				expect(emitter.listenerCount(eventName2)).to.equal(2);

				expect(captures).to.deep.equal({
					a: 1, b: 2, c: 1, d: 2, e: 1
				});
			};

			it('invokes handler only for subscribed channel', () => {
				// Assert:
				assertOnlySubscribedHandlersAreInvoked('block', 'confirmedAdded', 2);
			});

			it('invokes handler only for subscribed channel or subevent', () => {
				// Assert:
				assertOnlySubscribedHandlersAreInvoked('block', 'block.close', 1);
			});

			it('registers handler for propagating channel close event', () => {
				// Arrange:
				const context = {};
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocketWithCapture(context));

				// - set up subscriptions to two channels and one subevent
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});

				// - register close events
				const captures = {};
				emitter.on('block.close', () => { captures.block = true; });
				emitter.on('confirmedAdded.close', () => { captures.confirmedAdded = true; });

				// Act: simulate close of block zsocket
				context.zsockets.block.eventHandlers.zsocket_close[0]();

				// Assert: channel close event should have been raised for block channel only
				expect(captures).to.deep.equal({ block: true });

				// - close has also removed related socket reference and listeners
				expect(emitter.zsocketCount()).to.equal(1);
				expect(emitter.listenerCount('block')).to.equal(0);
				expect(emitter.listenerCount('block.close')).to.equal(0);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(2);

				// - close has not closed the socket because it is assumed to have been closed prior to the event
				expect(context.zsockets.block.numCloseCalls).to.equal(0);
				expect(context.zsockets.confirmedAdded.numCloseCalls).to.equal(0);
			});
		});

		describe('removeAllListeners', () => {
			it('has no effect when no matching subscribers are present', () => {
				// Arrange:
				const context = {};
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocketWithCapture(context));
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});

				// Act:
				emitter.removeAllListeners('unconfirmedAdded');

				// Assert:
				expect(emitter.zsocketCount()).to.equal(2);
				expect(emitter.listenerCount('block')).to.equal(2);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(2);

				expect(context.zsockets.block.numCloseCalls).to.equal(0);
				expect(context.zsockets.confirmedAdded.numCloseCalls).to.equal(0);
			});

			it('removes all matching channel subscribers', () => {
				// Arrange:
				const context = {};
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocketWithCapture(context));
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});

				// Act:
				emitter.removeAllListeners('block');

				// Assert:
				expect(emitter.zsocketCount()).to.equal(1);
				expect(emitter.listenerCount('block')).to.equal(0);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(2);

				expect(context.zsockets.block.numCloseCalls).to.equal(1);
				expect(context.zsockets.confirmedAdded.numCloseCalls).to.equal(0);
			});

			it('removes all matching and subevent subscribers', () => {
				// Arrange:
				const context = {};
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocketWithCapture(context));
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});

				// Act:
				emitter.removeAllListeners('block');

				// Assert:
				expect(emitter.zsocketCount()).to.equal(1);
				expect(emitter.listenerCount('block')).to.equal(0);
				expect(emitter.listenerCount('block.close')).to.equal(0);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(2);

				expect(context.zsockets.block.numCloseCalls).to.equal(1);
				expect(context.zsockets.confirmedAdded.numCloseCalls).to.equal(0);
			});

			it('fails when attempting to remove listeners for any subevent', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});

				// Act + Assert: removal attempts for both supported and unsupported subevents should fail because there is no reason
				//               to remove subevent subscriptions while remaining subscribed to the associated channel
				expect(() => emitter.removeAllListeners('block.close').to.throw('block.close must be a channel'));
				expect(() => emitter.removeAllListeners('block.foo').to.throw('block.foo must be a channel'));
			});

			it('allows new subscriptions for removed channels', () => {
				// Arrange:
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocket);
				emitter.on('block', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});

				// Act:
				emitter.removeAllListeners('block');
				emitter.on('block', () => {});

				// Assert:
				expect(emitter.zsocketCount()).to.equal(2);
				expect(emitter.listenerCount('block')).to.equal(1);
				expect(emitter.listenerCount('block.close')).to.equal(0);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(2);
			});
		});

		describe('close', () => {
			it('removes all channel and subevent subscribers', () => {
				// Arrange:
				const context = {};
				const emitter = zmqUtils.createMultisocketEmitter(createMockZsocketWithCapture(context));
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});
				emitter.on('block', () => {});
				emitter.on('block.close', () => {});
				emitter.on('confirmedAdded', () => {});

				// Act:
				emitter.close();

				// Assert:
				expect(emitter.zsocketCount()).to.equal(0);
				expect(emitter.listenerCount('block')).to.equal(0);
				expect(emitter.listenerCount('block.close')).to.equal(0);
				expect(emitter.listenerCount('confirmedAdded')).to.equal(0);

				expect(context.zsockets.block.numCloseCalls).to.equal(1);
				expect(context.zsockets.confirmedAdded.numCloseCalls).to.equal(1);
			});
		});
	});
});
