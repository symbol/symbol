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

import MessageChannelBuilder from '../../src/connection/MessageChannelBuilder.js';
import createZmqConnectionService, { ZmqSocketWrapper } from '../../src/connection/zmqService.js';
import test from '../testUtils.js';
import { expect } from 'chai';
import { Address } from 'symbol-sdk/symbol';
import zmq from 'zeromq';

describe('ZmqSocketWrapper', () => {
	const createMockSubscriber = () => {
		const eventsPendingResolvers = [];
		const receivePendingResolvers = [];

		const subscriber = {
			linger: undefined,
			closed: false,
			connectCalls: [],
			subscribeCalls: [],
			numCloseCalls: 0,
			events: {
				receive: () => new Promise((resolve, reject) => {
					eventsPendingResolvers.push({ resolve, reject });
				})
			},
			receive: () => new Promise((resolve, reject) => {
				receivePendingResolvers.push({ resolve, reject });
			}),
			connect: url => { subscriber.connectCalls.push(url); },
			subscribe: filter => { subscriber.subscribeCalls.push(filter); },
			close: () => {
				++subscriber.numCloseCalls;
				subscriber.closed = true;
				eventsPendingResolvers.splice(0).forEach(({ reject }) => reject(new Error('socket closed')));
				receivePendingResolvers.splice(0).forEach(({ reject }) => reject(new Error('socket closed')));
			},
			rejectEvent: err => {
				const pending = eventsPendingResolvers.shift();
				if (pending)
					pending.reject(err);
			},
			pushFrames: frames => {
				const pending = receivePendingResolvers.shift();
				if (pending)
					pending.resolve(frames);
			}
		};
		return subscriber;
	};

	const createWrapper = (key, subscriber) => new ZmqSocketWrapper(key, () => subscriber);

	let subscriber;
	let wrapper;

	beforeEach(() => {
		// Arrange:
		subscriber = createMockSubscriber();
		wrapper = createWrapper('test-key', subscriber);
	});

	afterEach(() => {
		if (!wrapper.innerSocket.closed)
			wrapper.close();
	});

	describe('constructor', () => {
		it('stores key and initializes inner socket with linger 0', () => {
			// Act: in the before each hook

			// Assert:
			expect(wrapper.eventsLoopActive).to.equal(false);
			expect(wrapper.innerSocket.linger).to.equal(0);
			expect(wrapper.key).to.equal('test-key');
		});
	});

	describe('connect', () => {
		it('delegates to inner socket', () => {
			// Act:
			wrapper.connect('tcp://127.0.0.1:7654');

			// Assert:
			expect(wrapper.innerSocket.connectCalls).to.deep.equal(['tcp://127.0.0.1:7654']);
		});
	});

	describe('subscribe', () => {
		it('delegates to inner socket', () => {
			// Arrange:
			const filter = Buffer.of(0x01, 0x02);

			// Act:
			wrapper.subscribe(filter);

			// Assert:
			expect(wrapper.innerSocket.subscribeCalls).to.deep.equal([filter]);
		});
	});

	describe('monitor', () => {
		it('emits error event when events loop throws while active', () => {
			// Arrange:
			wrapper.monitor();

			// Act:
			return new Promise(resolve => {
				wrapper.once('monitor:error', err => {
					// Assert:
					expect(err.message).to.equal('events loop error');
					resolve();
				});
				subscriber.rejectEvent(new Error('events loop error'));
			});
		});

		it('suppresses error event when events loop throws after unmonitor', async () => {
			// Arrange:
			wrapper.monitor();

			// Act: stop monitoring before the error arrives
			wrapper.unmonitor();
			const emittedErrors = [];
			wrapper.on('error', err => emittedErrors.push(err));
			subscriber.rejectEvent(new Error('late error'));

			// Wait for async processing
			await new Promise(resolve => { setTimeout(resolve, 10); });

			// Assert: no error was emitted
			expect(emittedErrors).to.deep.equal([]);
		});

		it('stops events loop', () => {
			// Arrange:
			wrapper.monitor();

			// Act:
			wrapper.unmonitor();

			// Assert:
			expect(wrapper.eventsLoopActive).to.equal(false);
		});
	});

	describe('messaging', () => {
		const testMessageReceived = shouldSubscribe => {
			// Arrange:
			const receivedArgs = [];
			const frame1 = Buffer.from('topic');
			const frame2 = Buffer.from('data');
			let expectedReceivedLength = 0;
			wrapper.on('message', (...args) => receivedArgs.push(args));
			if (shouldSubscribe) {
				expectedReceivedLength = 1;
				wrapper.subscribe('');
			}

			// Act:
			return new Promise(resolve => {
				subscriber.pushFrames([frame1, frame2]);
				setTimeout(() => {
					// Assert:
					expect(receivedArgs).to.have.lengthOf(expectedReceivedLength);
					if (shouldSubscribe)
						expect(receivedArgs[0]).to.deep.equal([frame1, frame2]);
					else
						expect(receivedArgs).to.deep.equal([]);

					wrapper.close();
					resolve();
				}, 0);
			});
		};

		it('calls handler with spread frames for each received message', () => testMessageReceived(true));

		it('handler is not called if not subscribe', () => testMessageReceived(false));

		it('silently stops when inner socket is closed', async () => {
			// Arrange:
			let handlerCallCount = 0;
			wrapper.subscribe('');
			wrapper.on('message', () => { ++handlerCallCount; });

			// Act: close immediately (rejects the pending receive)
			subscriber.close();
			subscriber.pushFrames([Buffer.from('after close')]);

			// Wait for async processing
			await new Promise(resolve => { setTimeout(resolve, 10); });

			// Assert: handler was never called, no error thrown
			expect(handlerCallCount).to.equal(0);
		});
	});

	describe('close', () => {
		it('closes inner socket and stops events loop', () => {
			// Arrange:
			wrapper.monitor();

			// Act:
			wrapper.close();

			// Assert:
			expect(wrapper.eventsLoopActive).to.equal(false);
			expect(wrapper.innerSocket.numCloseCalls).to.equal(1);
		});

		it('ignores errors from inner socket close', () => {
			// Arrange:
			wrapper.innerSocket.close = () => { throw new Error('close failed'); };

			// Act + Assert: should not throw
			expect(() => wrapper.close()).not.to.throw();
		});
	});
});

describe('zmq service', () => {
	const cleanupActions = [];
	afterEach(() => {
		// close zmq sockets used during the previous test
		while (0 < cleanupActions.length) {
			const action = cleanupActions.pop();
			action();
		}
	});

	const createDefaultZmqConnectionService = () => {
		const zmqConfig = {
			host: '127.0.0.1', port: '3333', connectTimeout: 10
		};
		const channelDescriptors = new MessageChannelBuilder().build();
		const service = createZmqConnectionService(zmqConfig, channelDescriptors, test.createMockLogger());
		cleanupActions.push(() => service.close());
		return service;
	};

	const createRandomAddressString = () => new Address(test.random.address()).toString();

	describe('invalid subscription', () => {
		const assertInvalidSubscription = (channel, error) => {
			// Arrange: notice that these tests should fail before creating a subscriber
			const service = createDefaultZmqConnectionService();

			// Assert:
			expect(() => service.on(channel, () => {})).to.throw(error);

			// Sanity:
			expect(service.zsocketCount()).to.equal(0);
		};

		it('throws if category has no associated channel descriptor', () => {
			// Assert:
			assertInvalidSubscription('foo', 'unknown topic category foo');
		});

		it('throws if category filter cannot be created due to invalid param', () => {
			// Assert:
			assertInvalidSubscription('block/12345', 'unexpected param to block subscription');
		});
	});

	describe('valid subscriptions', () => {
		it('creates new socket for new topic', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();

			// Act:
			service.on('block', () => {});

			// Assert:
			expect(service.zsocketCount()).to.equal(1);
			expect(service.listenerCount('block')).to.equal(1);
		});

		it('creates socket per topic', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();
			const address = createRandomAddressString();

			// Act:
			service.on('block', () => {});
			service.on(`confirmedAdded/${address}`, () => {});
			service.on(`unconfirmedAdded/${address}`, () => {});

			// Assert:
			expect(service.zsocketCount()).to.equal(3);
			expect(service.listenerCount('block')).to.equal(1);
			expect(service.listenerCount(`confirmedAdded/${address}`)).to.equal(1);
			expect(service.listenerCount(`unconfirmedAdded/${address}`)).to.equal(1);
		});

		it('reuses socket for existing topic', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();

			// Act:
			for (let i = 0; 9 > i; ++i)
				service.on('block', () => {});

			// Assert:
			expect(service.zsocketCount()).to.equal(1);
			expect(service.listenerCount('block')).to.equal(9);
		});

		it('raises channel close event on connection timeout', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();
			return new Promise(resolve => {
				service.on('block.close', () => {
					// Assert: socket is already closed when event is raised
					expect(service.zsocketCount()).to.equal(0);

					setTimeout(() => {
						// - listeners are removed after short delay
						expect(service.listenerCount('block')).to.equal(0);
						expect(service.listenerCount('block.close')).to.equal(0);
						resolve();
					}, 0);
				});

				// Act:
				service.on('block', () => {});
			});
		});
	});

	describe('subscription messages', () => {
		const generateBlockBuffers = () => ({
			block: test.createSampleBlock().buffer,
			entityHash: Buffer.from(test.random.hash()),
			generationHash: Buffer.from(test.random.hash())
		});

		it('forwards messages to subscribed handlers', () => {
			// Arrange:
			const zmqConfig = {
				host: '127.0.0.1', port: '3333', connectTimeout: 1000
			};
			const channelDescriptors = new MessageChannelBuilder().build();
			const service = createZmqConnectionService(zmqConfig, channelDescriptors, test.createLogger());
			cleanupActions.push(() => service.close());

			const blockBuffers = generateBlockBuffers();
			return new Promise(resolve => {
				// Arrange: create a publisher and publish a block
				const endpoint = `tcp://${zmqConfig.host}:${zmqConfig.port}`;
				const zsocket = new zmq.Publisher();
				zsocket.linger = 0;
				cleanupActions.push(() => { zsocket.close(); });

				zsocket.bind(endpoint).then(() => {
					// Arrange: subscribe to block events (this needs to be done after bind in order to avoid potential races)
					service.on('block', message => {
						// Assert: the parsed message is consistent with the published block message
						//         since formatting is not configured, meta properties are raw values
						expect(message).to.deep.equal({
							type: 'blockHeaderWithMetadata',
							payload: {
								block: test.createSampleBlock().model,
								meta: { hash: blockBuffers.entityHash, generationHash: blockBuffers.generationHash }
							}
						});
						resolve();
					});

					// Act: publish a single block (as a multipart message) after completion of bind callback processing
					setTimeout(async () => {
						const marker = Buffer.of(0x49, 0x6A, 0xCA, 0x80, 0xE4, 0xD8, 0xF2, 0x9F);
						await zsocket.send([marker, blockBuffers.block, blockBuffers.entityHash, blockBuffers.generationHash]);
					}, 100);
				});
			});
		});
	});

	describe('remove all listeners', () => {
		it('removes all subscriptions for topic', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();
			const address1 = createRandomAddressString();
			const address2 = createRandomAddressString();

			// - add subscriptions
			service.on(`confirmedAdded/${address1}`, () => {});
			service.on('block', () => {});
			service.on(`confirmedAdded/${address1}.close`, () => {});
			service.on(`confirmedAdded/${address1}`, () => {});
			service.on(`confirmedAdded/${address2}`, () => {});

			// Act:
			service.removeAllListeners(`confirmedAdded/${address1}`);

			// Assert:
			expect(service.zsocketCount()).to.equal(2);
			expect(service.listenerCount('block')).to.equal(1);
			expect(service.listenerCount(`confirmedAdded/${address1}`)).to.equal(0);
			expect(service.listenerCount(`confirmedAdded/${address1}.close`)).to.equal(0);
			expect(service.listenerCount(`confirmedAdded/${address2}`)).to.equal(1);
		});

		it('is idempotent', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();
			const address = createRandomAddressString();

			// - add subscriptions
			service.on(`confirmedAdded/${address}`, () => {});
			service.on('block', () => {});
			service.on(`confirmedAdded/${address}.close`, () => {});
			service.on(`confirmedAdded/${address}`, () => {});

			// Act:
			for (let i = 0; 9 > i; ++i)
				service.removeAllListeners(`confirmedAdded/${address}`);

			// Assert:
			expect(service.zsocketCount()).to.equal(1);
			expect(service.listenerCount('block')).to.equal(1);
			expect(service.listenerCount(`confirmedAdded/${address}`)).to.equal(0);
			expect(service.listenerCount(`confirmedAdded/${address}.close`)).to.equal(0);
		});

		it('allows new subscriptions to previously removed topics', () => {
			// Arrange:
			const service = createDefaultZmqConnectionService();
			const address = createRandomAddressString();

			// - add subscriptions
			service.on(`confirmedAdded/${address}`, () => {});
			service.on('block', () => {});
			service.on(`confirmedAdded/${address}.close`, () => {});
			service.on(`confirmedAdded/${address}`, () => {});

			// Act: remove listeners and then add one
			service.removeAllListeners(`confirmedAdded/${address}`);
			service.on(`confirmedAdded/${address}`, () => {});

			// Assert:
			expect(service.zsocketCount()).to.equal(2);
			expect(service.listenerCount('block')).to.equal(1);
			expect(service.listenerCount(`confirmedAdded/${address}`)).to.equal(1);
			expect(service.listenerCount(`confirmedAdded/${address}.close`)).to.equal(0);
		});
	});
});
