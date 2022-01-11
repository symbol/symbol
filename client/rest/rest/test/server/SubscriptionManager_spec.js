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

const SubscriptionManager = require('../../src/server/SubscriptionManager');
const { expect } = require('chai');

describe('subscription manager', () => {
	const createSubscription = (channel, client) => ({ channel, client });

	const createChannelSubscriptions = (channelName, startId, clientCount) => {
		let id = startId;
		return Array.from(Array(clientCount)).map(() => { ++id; return createSubscription(channelName, `client${id}`); });
	};

	const createSubscriptions = (...descriptors) => {
		const current = [];
		descriptors.forEach(descriptor => {
			const [channelName, clientCount] = descriptor;
			current.push(...createChannelSubscriptions(channelName, 0, clientCount));
		});

		return current;
	};

	const addAllSubscriptions = (manager, subscriptions) => {
		subscriptions.forEach(subscription => {
			manager.add(subscription.channel, subscription.client);
		});
	};

	const runTest = (subscriptions, action, assertCaptures) => {
		// Arrange:
		const captures = { newChannels: [], clients: [], removal: [] };
		const manager = new SubscriptionManager({
			newChannel: (channel, subscribers) => captures.newChannels.push({ channel, subscribers }),
			removeChannel: channel => captures.removal.push(channel),
			newClient: (channel, client) => captures.clients.push({ channel, client })
		});

		// Act:
		action(manager, subscriptions);

		// Assert: common for all tests: all subscriptions are always subscribed by manager (see addAllSubscriptions)
		expect(captures.clients).to.deep.equal(subscriptions);
		assertCaptures(captures, manager.subscriptions);
	};

	const assertChannels = (newChannelCaptures, subscriptions, expectedChannelNames) => {
		// Assert:
		const channelNames = newChannelCaptures.map(capture => capture.channel);
		expect(channelNames).to.deep.equal(expectedChannelNames);

		// note: subscriptions passed to assertChannels are not original subscriptions, but SubscriptionManager owned subscriptions.
		// assert that proper set of clients has been passed in invocation of new channel handler
		for (let id = 0; expectedChannelNames.length > id; ++id) {
			const channelName = expectedChannelNames[id];

			// if channel is not in subscriptions it means it's been deleted, and we have nothing to compare against
			if (channelName in subscriptions)
				expect(newChannelCaptures[id].subscribers).to.equal(subscriptions[channelName]);
		}
	};

	const assertChannelSubscribers = (newChannels, expectedClientNames) => {
		// Assert:
		expect(Array.from(newChannels.subscribers)).to.deep.equal(expectedClientNames);
	};

	// region basic

	it('is initially empty', () => {
		// Arrange:
		const manager = new SubscriptionManager({});

		// Assert:
		expect(manager.subscriptions).to.deep.equal({});
	});

	// endregion

	// region add

	it('can subscribe a single client to single channel', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 1]),
			addAllSubscriptions,
			(captures, subscriptions) => {
				// Assert:
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1']);
				assertChannelSubscribers(captures.newChannels[0], ['client1']);
			}
		);
	});

	it('can subscribe a single client to the same channel multiple times', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 1]),
			(...args) => {
				// Act: subscribe to the same channel multiple times
				for (let i = 0; 5 > i; ++i)
					addAllSubscriptions(...args);
			},
			(captures, subscriptions) => {
				// Assert: only unique subscriptions are present
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1']);
				assertChannelSubscribers(captures.newChannels[0], ['client1']);
			}
		);
	});

	it('can subscribe multiple clients to single channel', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3]),
			addAllSubscriptions,
			(captures, subscriptions) => {
				// Assert:
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1']);
				assertChannelSubscribers(captures.newChannels[0], ['client1', 'client2', 'client3']);
			}
		);
	});

	it('can subscribe multiple clients to multiple clients', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3], ['channel-2', 5]),
			addAllSubscriptions,
			(captures, subscriptions) => {
				// Assert:
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1', 'channel-2']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1', 'channel-2']);
				assertChannelSubscribers(captures.newChannels[0], ['client1', 'client2', 'client3']);
				assertChannelSubscribers(captures.newChannels[1], ['client1', 'client2', 'client3', 'client4', 'client5']);
			}
		);
	});

	it('cannot subscribe clients to an unsupported channel', () => {
		// Arrange: configure newChannel to throw to simulate unsupported channel
		const manager = new SubscriptionManager({
			newChannel: channel => { throw Error(`channel ${channel} is not supported`); }
		});

		// Act: add disallowed subscriptions (all attempts should fail)
		expect(() => manager.add('foo')).to.throw('channel foo is not supported');
		expect(() => manager.add('foo')).to.throw('channel foo is not supported');

		// Assert: no subscriptions were added
		expect(manager.subscriptions).to.deep.equal({});
	});

	// endregion

	// region delete

	it('can unsubscribe a client from a channel without affecting other client subscriptions', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3], ['channel-2', 5]),
			(manager, subscriptions) => {
				// Act: delete a single subscription
				addAllSubscriptions(manager, subscriptions);
				manager.delete('channel-1', 'client2');
			},
			(captures, subscriptions) => {
				// Assert: no removals should have been raised because all channels are still active
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1', 'channel-2']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1', 'channel-2']);
				assertChannelSubscribers(captures.newChannels[0], ['client1', 'client3']);
				assertChannelSubscribers(captures.newChannels[1], ['client1', 'client2', 'client3', 'client4', 'client5']);
			}
		);
	});

	it('can unsubscribe all clients from a channel without affecting other client subscriptions', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3], ['channel-2', 5]),
			(manager, subscriptions) => {
				// Act: delete all channel-1 subscriptions
				addAllSubscriptions(manager, subscriptions);
				manager.delete('channel-1', 'client2');
				manager.delete('channel-1', 'client1');
				manager.delete('channel-1', 'client3');
			},
			(captures, subscriptions) => {
				// Assert:
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-2']);

				expect(captures.removal).to.deep.equal(['channel-1']);
				assertChannels(captures.newChannels, subscriptions, ['channel-1', 'channel-2']);
				assertChannelSubscribers(captures.newChannels[0], []);
				assertChannelSubscribers(captures.newChannels[1], ['client1', 'client2', 'client3', 'client4', 'client5']);
			}
		);
	});

	it('can unsubscribe a client from an unknown channel', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3], ['channel-2', 5]),
			(manager, subscriptions) => {
				// Act: delete a single subscription for an unsubscribed channel
				addAllSubscriptions(manager, subscriptions);
				manager.delete('channel-X', 'client2');
			},
			(captures, subscriptions) => {
				// Assert: nothing should have changed
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1', 'channel-2']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1', 'channel-2']);
				assertChannelSubscribers(captures.newChannels[0], ['client1', 'client2', 'client3']);
				assertChannelSubscribers(captures.newChannels[1], ['client1', 'client2', 'client3', 'client4', 'client5']);
			}
		);
	});

	// endregion

	// region clientSubscriptions

	it('can get client subscriptions for known clients', () => {
		// Arrange:
		const manager = new SubscriptionManager({ newChannel: () => {} });
		const subscriptions = createSubscriptions(['channel-1', 3], ['channel-2', 5], ['channel-3', 1]);

		// Act:
		addAllSubscriptions(manager, subscriptions);

		// Assert:
		expect(manager.clientSubscriptions('client1')).to.deep.equal(['channel-1', 'channel-2', 'channel-3']);
		expect(manager.clientSubscriptions('client2')).to.deep.equal(['channel-1', 'channel-2']);
		expect(manager.clientSubscriptions('client3')).to.deep.equal(['channel-1', 'channel-2']);
		expect(manager.clientSubscriptions('client4')).to.deep.equal(['channel-2']);
		expect(manager.clientSubscriptions('client5')).to.deep.equal(['channel-2']);
	});

	it('can get client subscriptions for unknown clients', () => {
		// Arrange:
		const manager = new SubscriptionManager({ newChannel: () => {} });
		const subscriptions = createSubscriptions(['channel-1', 3], ['channel-2', 5], ['channel-3', 1]);

		// Act:
		addAllSubscriptions(manager, subscriptions);

		// Assert:
		expect(manager.clientSubscriptions('clientX')).to.deep.equal([]);
	});

	it('can get client subscriptions after subscriptions and unsubscriptions', () => {
		// Arrange:
		const manager = new SubscriptionManager({ newChannel: () => {} });
		const subscriptions = createSubscriptions(['channel-1', 3], ['channel-2', 5], ['channel-3', 1]);

		// Act:
		addAllSubscriptions(manager, subscriptions);
		manager.delete('channel-1', 'client1');
		manager.delete('channel-2', 'client3');
		manager.delete('channel-2', 'client4');

		// Assert:
		expect(manager.clientSubscriptions('client1')).to.deep.equal(['channel-2', 'channel-3']);
		expect(manager.clientSubscriptions('client2')).to.deep.equal(['channel-1', 'channel-2']);
		expect(manager.clientSubscriptions('client3')).to.deep.equal(['channel-1']);
		expect(manager.clientSubscriptions('client4')).to.deep.equal([]);
		expect(manager.clientSubscriptions('client5')).to.deep.equal(['channel-2']);
	});

	// endregion

	// region deleteClient

	it('can delete a client from all subscriptions', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3], ['channel-2', 5]),
			(manager, subscriptions) => {
				// Act: delete a single client
				addAllSubscriptions(manager, subscriptions);
				manager.deleteClient('client2');
			},
			(captures, subscriptions) => {
				// Assert: no removals should have been raised because all channels are still active
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-1', 'channel-2']);

				expect(captures.removal).to.deep.equal([]);
				assertChannels(captures.newChannels, subscriptions, ['channel-1', 'channel-2']);
				assertChannelSubscribers(captures.newChannels[0], ['client1', 'client3']);
				assertChannelSubscribers(captures.newChannels[1], ['client1', 'client3', 'client4', 'client5']);
			}
		);
	});

	it('can delete all clients subscribed to a channel', () => {
		// Arrange:
		runTest(
			createSubscriptions(['channel-1', 3], ['channel-2', 5]),
			(manager, subscriptions) => {
				// Act: delete all clients
				addAllSubscriptions(manager, subscriptions);
				manager.deleteClient('client1');
				manager.deleteClient('client2');
				manager.deleteClient('client3');
			},
			(captures, subscriptions) => {
				// Assert:
				expect(Object.keys(subscriptions)).to.deep.equal(['channel-2']);

				expect(captures.removal).to.deep.equal(['channel-1']);
				assertChannels(captures.newChannels, subscriptions, ['channel-1', 'channel-2']);
				assertChannelSubscribers(captures.newChannels[0], []);
				assertChannelSubscribers(captures.newChannels[1], ['client4', 'client5']);
			}
		);
	});

	// endregion
});
