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

const websocketUtils = require('../../src/server/websocketUtils');
const { expect } = require('chai');

describe('websocketUtils', () => {
	const createMockSender = (options = {}) => {
		const sender = {
			payloads: [],
			numCloseCalls: 0,
			eventHandlers: {}
		};

		sender.send = (data, cb) => {
			sender.payloads.push(data);
			cb(options.raiseSendError ? new Error('send error') : undefined);
		};

		sender.close = () => { ++sender.numCloseCalls; };
		sender.on = (eventName, eventHandler) => { sender.eventHandlers[eventName] = eventHandler; };
		return sender;
	};

	describe('createMultisender', () => {
		describe('send', () => {
			it('does nothing when there are no subscribers', () => {
				// Arrange:
				const multisender = websocketUtils.createMultisender('topicName', [], value => ({ str: value.toString(), value }));

				// Act + Assert: no exceptions
				multisender.send(123);
			});

			it('forwards formatted data to single subscriber', () => {
				// Arrange:
				const sender = createMockSender();
				const multisender = websocketUtils.createMultisender('topicName', [sender], value => ({ str: value.toString(), value }));

				// Act:
				multisender.send(123);

				// Assert:
				expect(sender.payloads).to.deep.equal([{ str: '123', value: 123 }]);
				expect(sender.numCloseCalls).to.equal(0);
			});

			it('forwards formatted data to multiple subscribers', () => {
				// Arrange:
				const senders = [createMockSender(), createMockSender(), createMockSender()];
				const multisender = websocketUtils.createMultisender('topicName', senders, value => ({ str: value.toString(), value }));

				// Act:
				multisender.send(123);

				// Assert:
				senders.forEach((sender, index) => {
					const message = `sender ${index}`;
					expect(sender.payloads, message).to.deep.equal([{ str: '123', value: 123 }]);
					expect(sender.numCloseCalls).to.equal(0);
				});
			});

			it('closes subscriber on send error', () => {
				// Arrange:
				const senders = [createMockSender(), createMockSender({ raiseSendError: true }), createMockSender()];
				const multisender = websocketUtils.createMultisender('topicName', senders, value => ({ str: value.toString(), value }));

				// Act:
				multisender.send(123);

				// Assert: only the second sender should have been closed
				senders.forEach((sender, index) => {
					const message = `sender ${index}`;
					expect(sender.payloads, message).to.deep.equal([{ str: '123', value: 123 }]);
					expect(sender.numCloseCalls, message).to.equal(1 === index ? 1 : 0);
				});
			});
		});

		describe('close', () => {
			it('does nothing when there are no subscribers', () => {
				// Arrange:
				const multisender = websocketUtils.createMultisender('topicName', []);

				// Act + Assert: no exceptions
				multisender.close();
			});

			it('closes all subscribers when there is single subscriber', () => {
				// Arrange:
				const sender = createMockSender();
				const multisender = websocketUtils.createMultisender('topicName', [sender]);

				// Act:
				multisender.close();

				// Assert:
				expect(sender.numCloseCalls).to.equal(1);
			});

			it('closes all subscribers when there are multiple subscribers', () => {
				// Arrange:
				const senders = [createMockSender(), createMockSender(), createMockSender()];
				const multisender = websocketUtils.createMultisender('topicName', senders);

				// Act:
				multisender.close();

				// Assert:
				senders.forEach((sender, index) => {
					expect(sender.numCloseCalls, `sender ${index}`).to.equal(1);
				});
			});
		});
	});

	describe('handshake', () => {
		it('assigns client unique id', () => {
			// Arrange:
			const client1 = createMockSender();
			const client2 = createMockSender();

			// Act:
			websocketUtils.handshake(client1);
			websocketUtils.handshake(client2);

			// Assert:
			expect(client1.uid.length).to.equal(32);
			expect(client2.uid.length).to.equal(32);
			expect(client1.uid).to.not.deep.equal(client2.uid);
		});

		it('sends unique id to client', () => {
			// Arrange:
			const client = createMockSender();

			// Act:
			websocketUtils.handshake(client);

			// Assert:
			expect(client.payloads).to.deep.equal([`{"uid": "${client.uid}"}`]);
			expect(client.numCloseCalls).to.equal(0);
		});

		it('closes client on send error', () => {
			// Arrange:
			const client = createMockSender({ raiseSendError: true });

			// Act:
			websocketUtils.handshake(client);

			// Assert:
			expect(client.payloads).to.deep.equal([`{"uid": "${client.uid}"}`]);
			expect(client.numCloseCalls).to.equal(1);
		});

		it('closes client on arbitrary error', () => {
			// Arrange:
			const client = createMockSender();

			// Act:
			websocketUtils.handshake(client);
			client.eventHandlers.error(new Error());

			// Assert:
			expect(client.numCloseCalls).to.equal(1);
		});

		it('forwards message to message handler', () => {
			// Arrange:
			const client = createMockSender();
			const messages = [];

			// Act:
			websocketUtils.handshake(client, message => {
				messages.push(message);
				return undefined;
			});
			client.eventHandlers.message('hello world');

			// Assert:
			expect(messages).to.deep.equal(['hello world']);
			expect(client.numCloseCalls).to.equal(0);
		});

		it('closes client on message handler error', () => {
			// Arrange:
			const client = createMockSender();
			const messages = [];

			// Act:
			websocketUtils.handshake(client, message => {
				messages.push(message);
				return ['message handler error', new Error('cause')];
			});
			client.eventHandlers.message('hello world');

			// Assert:
			expect(messages).to.deep.equal(['hello world']);
			expect(client.numCloseCalls).to.equal(1);
		});
	});
});
