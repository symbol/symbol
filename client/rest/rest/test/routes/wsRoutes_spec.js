const { expect } = require('chai');
const wsRoutes = require('../../src/routes/wsRoutes');
const test = require('./utils/routeTestUtils');

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
