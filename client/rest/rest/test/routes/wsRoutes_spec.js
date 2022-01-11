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

const { test } = require('./utils/routeTestUtils');
const wsRoutes = require('../../src/routes/wsRoutes');
const { expect } = require('chai');

describe('web socket routes', () => {
	const setupWebsocketTest = (action, assertCaptures) => {
		// Arrange:
		const service = { eventHandlers: {}, removedChannels: [] };
		service.on = (eventName, eventHandler) => { service.eventHandlers[eventName] = eventHandler; };
		service.removeAllListeners = channel => { service.removedChannels.push(channel); };

		const routes = [];
		const server = test.setup.createMockServer('ws', routes);
		wsRoutes.register(server, undefined, { zmqService: service });

		// Act: get the desired route and pass it to action
		const route = test.setup.findRoute(routes, '/ws');
		action(route);

		// Assert:
		assertCaptures(service);
	};

	describe('newChannel', () => {
		const assertSubscriptionToEvent = (channel, expectedEvent) => {
			// Act:
			setupWebsocketTest(
				route => route.newChannel(channel, {}),
				service => {
					// Assert: channel event was registered
					const handler = service.eventHandlers[expectedEvent];
					expect(handler).to.be.a('function');
				}
			);
		};

		it('subscribes to service channel event', () => { assertSubscriptionToEvent('block', 'block'); });
		it('subscribes to service channel close event', () => { assertSubscriptionToEvent('block', 'block.close'); });

		it('handles service channel event by forwarding message to sender', () => {
			// Arrange:
			const message = {};
			const sendMessages = [];
			const sender = { send: sendMessage => sendMessages.push(sendMessage) };

			// Act:
			setupWebsocketTest(
				route => route.newChannel('block', sender),
				service => {
					// - invoke the handler
					const handler = service.eventHandlers.block;
					expect(handler).to.be.a('function');
					handler(message);

					// Assert: message was passed to handler
					expect(sendMessages.length).to.equal(1);
					expect(sendMessages[0]).to.equal(message);
				}
			);
		});

		it('handles service channel close event by closing sender', () => {
			// Arrange:
			let numCloseCalls = 0;
			const sender = { close: () => { ++numCloseCalls; } };

			// Act:
			setupWebsocketTest(
				route => route.newChannel('block', sender),
				service => {
					// - invoke the handler
					const handler = service.eventHandlers['block.close'];
					expect(handler).to.be.a('function');
					handler();

					// Assert: close was called on the sender
					expect(numCloseCalls).to.equal(1);
				}
			);
		});
	});

	describe('removeChannel', () => {
		it('forwards channel to service', () => {
			// Act:
			setupWebsocketTest(
				route => route.removeChannel('block'),
				service => {
					// Assert:
					expect(service.removedChannels).to.deep.equal(['block']);
				}
			);
		});
	});
});
