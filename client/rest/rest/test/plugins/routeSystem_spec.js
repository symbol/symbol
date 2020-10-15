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

const routeSystem = require('../../src/plugins/routeSystem');
const { test } = require('../routes/utils/routeTestUtils');
const { expect } = require('chai');

describe('route system', () => {
	const servicesTemplate = { config: { websocket: {} }, connections: {} };
	const configureTrailingParameters = [{ put: () => {} }, {}, servicesTemplate];

	it('cannot register unknown extension', () => {
		// Act:
		expect(() => routeSystem.configure(['transfer', 'foo', 'namespace'], {}, {}, servicesTemplate))
			.to.throw('plugin \'foo\' not supported');
	});

	it('has support for all extensions', () => {
		// Act:
		const supportedPluginNames = routeSystem.supportedPluginNames();

		// Assert:
		expect(supportedPluginNames).to.deep.equal([
			'accountLink',
			'aggregate',
			'lockHash',
			'lockSecret',
			'metadata',
			'mosaic',
			'multisig',
			'namespace',
			'receipts',
			'restrictions',
			'transfer'
		]);
	});

	describe('routes', () => {
		it('does not register default routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);

			// Act:
			routeSystem.configure([], server, {}, servicesTemplate);

			// Assert:
			expect(routes.length).to.equal(0);
		});

		it('can register single extension', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);
			const db = { namespaceById: () => Promise.resolve({}) };

			// Act:
			routeSystem.configure(['namespace'], server, db, servicesTemplate);

			// Assert:
			expect(routes).to.include('/namespaces/:namespaceId');
		});

		it('can register multiple extensions', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('get', routes);
			const db = { namespaceById: () => Promise.resolve({}) };

			// Act:
			routeSystem.configure(['namespace', 'transfer'], server, db, servicesTemplate);

			// Assert:
			expect(routes).to.include('/namespaces/:namespaceId');
		});

		it('can register single extension with service dependencies', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('put', routes);

			// Act: pass down required services too
			routeSystem.configure(['aggregate'], server, {}, servicesTemplate);

			// Assert:
			expect(routes).to.include('/transactions/partial');
		});
	});

	describe('transaction states', () => {
		it('can register single extension without custom transaction states', () => {
			// Act:
			const { transactionStates } = routeSystem.configure(['transfer'], ...configureTrailingParameters);

			// Assert:
			expect(transactionStates.length).to.equal(0);
		});

		it('can register single extension with custom transaction states', () => {
			// Act:
			const { transactionStates } = routeSystem.configure(['aggregate'], ...configureTrailingParameters);

			// Assert:
			expect(transactionStates.length).to.equal(1);
		});
	});

	describe('message channels', () => {
		it('can register single extension without custom message channels', () => {
			// Act:
			const { messageChannelDescriptors } = routeSystem.configure(['transfer'], ...configureTrailingParameters);

			// Assert:
			expect(Object.keys(messageChannelDescriptors)).to.deep.equal([
				'block', 'finalizedBlock', 'confirmedAdded', 'unconfirmedAdded', 'unconfirmedRemoved', 'status'
			]);
		});

		it('can register single extension with custom message channels', () => {
			// Act:
			const { messageChannelDescriptors } = routeSystem.configure(['aggregate'], ...configureTrailingParameters);

			// Assert:
			expect(Object.keys(messageChannelDescriptors)).to.deep.equal([
				'block', 'finalizedBlock', 'confirmedAdded', 'unconfirmedAdded', 'unconfirmedRemoved', 'status',
				'partialAdded', 'partialRemoved', 'cosignature'
			]);
		});

		// following two tests are used to ensure configuration is passed down correctly to extension message channels
		it('extension filter rejects marker without topic param', () => {
			// Arrange:
			const { messageChannelDescriptors } = routeSystem.configure(['aggregate'], ...configureTrailingParameters);
			const { filter } = messageChannelDescriptors.partialAdded;

			// Act:
			expect(() => filter('')).to.throw('address param missing from address subscription');
		});

		it('extension filter accepts marker without topic param with allowOptionalAddress', () => {
			// Arrange:
			const services = { config: { websocket: { allowOptionalAddress: true } }, connections: {} };
			const { messageChannelDescriptors } = routeSystem.configure(['aggregate'], { put: () => {} }, {}, services);
			const { filter } = messageChannelDescriptors.partialAdded;

			// Act:
			const topic = filter('');

			// Assert:
			expect(topic.length).to.equal(1);
			expect(topic[0]).to.equal(0x70);
		});
	});
});
