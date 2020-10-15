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

const { ServerMessageHandler } = require('../../../src/connection/serverMessageHandlers');
const aggregate = require('../../../src/plugins/aggregate/aggregate');
const { test } = require('../../routes/utils/routeTestUtils');
const pluginTest = require('../utils/pluginTestUtils');
const { expect } = require('chai');

describe('aggregate plugin', () => {
	pluginTest.assertThat.pluginDoesNotCreateDb(aggregate);

	describe('register transaction states', () => {
		it('registers partial state', () => {
			// Arrange:
			const states = [];

			// Act:
			aggregate.registerTransactionStates(states);

			// Assert:
			expect(states.length).to.equal(1);
			expect(states[0]).to.deep.equal({ friendlyName: 'partial', dbPostfix: 'Partial', routePostfix: '/partial' });
		});
	});

	describe('register message channels', () => {
		const registerAndExtractChannelDescriptor = channelDescriptorName => {
			// Arrange:
			const channelDescriptors = [];
			const builder = { add: (name, markerChar, handler) => { channelDescriptors.push({ name, markerChar, handler }); } };

			// Act:
			aggregate.registerMessageChannels(builder);
			const channelDescriptor = channelDescriptors.find(descriptor => channelDescriptorName === descriptor.name);

			// Sanity:
			expect(channelDescriptors.length).to.equal(3);
			expect(channelDescriptor).to.not.equal(undefined);
			return channelDescriptor;
		};

		it('registers partialAdded', () => {
			// Act:
			const descriptor = registerAndExtractChannelDescriptor('partialAdded');

			// Assert:
			expect(descriptor).to.deep.equal({ name: 'partialAdded', markerChar: 'p', handler: ServerMessageHandler.transaction });
		});

		it('registers partialRemoved', () => {
			// Act:
			const descriptor = registerAndExtractChannelDescriptor('partialRemoved');

			// Assert:
			expect(descriptor).to.deep.equal({ name: 'partialRemoved', markerChar: 'q', handler: ServerMessageHandler.transactionHash });
		});

		it('registers cosignature', () => {
			// Act:
			const descriptor = registerAndExtractChannelDescriptor('cosignature');

			// Assert:
			expect(descriptor.name).to.equal('cosignature');
			expect(descriptor.markerChar).to.equal('c');
		});

		it('registers cosignature with handler that forwards to emit callback', () => {
			// Arrange:
			const emitted = [];
			const { handler } = registerAndExtractChannelDescriptor('cosignature');

			// Act:
			const buffer = Buffer.concat([
				Buffer.of(0x34, 0x54, 0x55, 0xFF, 0xFA, 0x0E, 0xCC, 0xB7),
				Buffer.alloc(test.constants.sizes.signerPublicKey, 33),
				Buffer.alloc(test.constants.sizes.signature, 44),
				Buffer.alloc(test.constants.sizes.hash256, 55)
			]);
			handler({}, eventData => emitted.push(eventData))(22, buffer, 99);

			// Assert:
			// - 22 is a "topic" so it's not forwarded
			// - trailing param 99 should be ignored
			expect(emitted.length).to.equal(1);
			expect(emitted[0]).to.deep.equal({
				type: 'aggregate.cosignature',
				payload: {
					version: [4283782196, 3083603706],
					signerPublicKey: Buffer.alloc(test.constants.sizes.signerPublicKey, 33),
					signature: Buffer.alloc(test.constants.sizes.signature, 44),
					parentHash: Buffer.alloc(test.constants.sizes.hash256, 55)
				}
			});
		});
	});

	describe('register routes', () => {
		it('registers aggregate PUT routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('put', routes);

			// Act:
			aggregate.registerRoutes(server, {}, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/transactions/cosignature',
				'/transactions/partial'
			]);
		});
	});
});
