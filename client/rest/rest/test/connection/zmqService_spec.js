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

const MessageChannelBuilder = require('../../src/connection/MessageChannelBuilder');
const { createZmqConnectionService } = require('../../src/connection/zmqService');
const test = require('../testUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const zmq = require('zeromq');

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
			host: '127.0.0.1', port: '3333', connectTimeout: 10, monitorInterval: 50
		};
		const channelDescriptors = new MessageChannelBuilder().build();
		const service = createZmqConnectionService(zmqConfig, {}, channelDescriptors, test.createMockLogger());
		cleanupActions.push(() => service.close());
		return service;
	};

	const createRandomAddressString = () => catapult.model.address.addressToString(test.random.address());

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
			block: Buffer.concat([
				Buffer.of(0x97, 0x87, 0x45, 0x0E, 0xE1, 0x6C, 0xB6, 0x62), // height 8b
				Buffer.of(0x30, 0x3A, 0x46, 0x8B, 0x15, 0x2D, 0x60, 0x54), // timestamp 8b
				Buffer.of(0x86, 0x02, 0x75, 0x30, 0xE8, 0x50, 0x78, 0xE8), // difficulty 8b
				Buffer.from(test.random.hash()), // previous block hash 32b
				Buffer.from(test.random.hash()), // block transactions hash 32b
				Buffer.from(test.random.hash()), // receiptsHashBuffer 32b
				Buffer.from(test.random.hash()), // stateHashBuffer 32b
				test.random.bytes(test.constants.sizes.addressDecoded), // beneficiaryAddress 24b
				Buffer.of(0x0A, 0x00, 0x00, 0x00), // fee multiplier 4b
				Buffer.of(0x00, 0x00, 0x00, 0x00) // reserved padding 4b
			]),

			entityHash: Buffer.from(test.random.hash()),
			generationHash: Buffer.from(test.random.hash())
		});

		it('forwards messages to subscribed handlers', () => {
			// Arrange:
			const zmqConfig = {
				host: '127.0.0.1', port: '3333', connectTimeout: 1000, monitorInterval: 50
			};
			const codec = {
				// - parsing is not being tested so just extract the entire parser contents into a buffer
				deserialize: parser => parser.buffer(parser.numUnprocessedBytes())
			};
			const channelDescriptors = new MessageChannelBuilder().build();
			const service = createZmqConnectionService(zmqConfig, codec, channelDescriptors, test.createLogger());
			cleanupActions.push(() => service.close());

			const blockBuffers = generateBlockBuffers();
			return new Promise((resolve, reject) => {
				// Arrange: create a publisher and publish a block
				const endpoint = `tcp://${zmqConfig.host}:${zmqConfig.port}`;
				const zsocket = zmq.socket('pub');
				cleanupActions.push(() => {
					zsocket.disconnect(endpoint);
					zsocket.close();
				});

				zsocket.bind(endpoint, err => {
					if (err) {
						reject(err);
						return;
					}

					// Arrange: subscribe to block events (this needs to be done after bind in order to avoid potential races)
					service.on('block', message => {
						// Assert: the parsed message is consistent with the published block message
						expect(message).to.deep.equal({
							type: 'blockHeaderWithMetadata',
							payload: {
								block: blockBuffers.block,
								meta: { hash: blockBuffers.entityHash, generationHash: blockBuffers.generationHash }
							}
						});
						resolve();
					});

					// Act: publish a single block (as a multipart message) after completion of bind callback processing
					setTimeout(() => {
						const marker = Buffer.of(0x49, 0x6A, 0xCA, 0x80, 0xE4, 0xD8, 0xF2, 0x9F);
						zsocket.send(marker, zmq.ZMQ_SNDMORE);
						zsocket.send(blockBuffers.block, zmq.ZMQ_SNDMORE);
						zsocket.send(blockBuffers.entityHash, zmq.ZMQ_SNDMORE);
						zsocket.send(blockBuffers.generationHash);
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
