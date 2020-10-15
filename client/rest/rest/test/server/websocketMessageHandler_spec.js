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

const websocketMessageHandler = require('../../src/server/websocketMessageHandler');
const { expect } = require('chai');

describe('websocketMessageHandler', () => {
	// region invalid

	const runInvalidMessageTest = traits => {
		// Arrange:
		const client = { uid: 'client1' };
		const messageJson = 'string' === typeof traits.message ? traits.message : JSON.stringify(traits.message);
		const subscriptionManager = {};

		// Act:
		const result = websocketMessageHandler.handleMessage(client, messageJson, subscriptionManager);

		// Assert:
		if (traits.hasCause) {
			expect(result.length).to.equal(2);
			expect(result[1]).to.not.equal(undefined);
		} else {
			expect(result.length).to.equal(1);
		}

		expect(result[0]).to.contain(traits.error);
	};

	it('rejects invalid json message', () => runInvalidMessageTest({
		message: 'hello', // non-json data
		error: 'parse error for data',
		hasCause: true
	}));

	it('rejects message without uid', () => runInvalidMessageTest({
		message: { subscribe: 'block' },
		error: 'client data does not have proper uid'
	}));

	it('rejects message with neither subscribe nor unsubscribe', () => runInvalidMessageTest({
		message: { uid: 'client1' },
		error: 'client subscription request (subscribe) must be string'
	}));

	it('rejects message with both subscribe and unsubscribe', () => runInvalidMessageTest({
		message: { uid: 'client1', subscribe: 'block', unsubscribe: 'block' },
		error: 'client data cannot specify both subscribe and unsubscribe'
	}));

	it('rejects message with malformed subscribe', () => runInvalidMessageTest({
		message: { uid: 'client1', subscribe: 7 },
		error: 'client subscription request (subscribe) must be string'
	}));

	it('rejects message with malformed unsubscribe', () => runInvalidMessageTest({
		message: { uid: 'client1', unsubscribe: 7 },
		error: 'client subscription request (unsubscribe) must be string'
	}));

	it('rejects message when subscription manager fails', () => runInvalidMessageTest({
		// subscriptionManager in runInvalidMessageTest does not support 'subscribe'
		message: { uid: 'client1', subscribe: 'block' },
		error: 'subscribe error for data',
		hasCause: true
	}));

	// endregion

	// region valid

	const createCapturingSubscriptionManager = () => {
		const captures = {
			adds: [],
			deletes: []
		};

		return {
			captures,
			add: (channel, client) => {
				captures.adds.push({ channel, client });
			},
			delete: (channel, client) => {
				captures.deletes.push({ channel, client });
			}
		};
	};

	it('can subscribe client to channel', () => {
		// Arrange:
		const client = { uid: 'client1' };
		const messageJson = JSON.stringify({ uid: 'client1', subscribe: 'block' });
		const subscriptionManager = createCapturingSubscriptionManager();

		// Act:
		const result = websocketMessageHandler.handleMessage(client, messageJson, subscriptionManager);

		// Assert:
		expect(result).to.equal(undefined);
		expect(subscriptionManager.captures.adds).to.deep.equal([{ channel: 'block', client }]);
		expect(subscriptionManager.captures.deletes).to.deep.equal([]);
	});

	it('can unsubscribe client from channel', () => {
		// Arrange:
		const client = { uid: 'client1' };
		const messageJson = JSON.stringify({ uid: 'client1', unsubscribe: 'block' });
		const subscriptionManager = createCapturingSubscriptionManager();

		// Act:
		const result = websocketMessageHandler.handleMessage(client, messageJson, subscriptionManager);

		// Assert:
		expect(result).to.equal(undefined);
		expect(subscriptionManager.captures.adds).to.deep.equal([]);
		expect(subscriptionManager.captures.deletes).to.deep.equal([{ channel: 'block', client }]);
	});

	// endregion
});
