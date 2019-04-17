/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const catapult = require('catapult-sdk');
const dbUtils = require('../../../src/db/dbUtils');
const mosaicRoutes = require('../../../src/plugins/routes/mosaicRoutes');
const sinon = require('sinon');
const test = require('../../routes/utils/routeTestUtils');

const { convertToLong } = dbUtils;
const { expect } = require('chai');

describe('mosaic routes', () => {
	describe('by id', () => {
		const mosaicIds = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const uint64MosaicIds = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];
		const errorMessage = 'has an invalid format';
		test.route.document.addGetPostDocumentRouteTests(mosaicRoutes.register, {
			routes: { singular: '/mosaic/:mosaicId', plural: '/mosaic' },
			inputs: {
				valid: { object: { mosaicId: mosaicIds[0] }, parsed: [uint64MosaicIds[0]], printable: mosaicIds[0] },
				validMultiple: { object: { mosaicIds }, parsed: uint64MosaicIds },
				invalid: { object: { mosaicId: '12345' }, error: `mosaicId ${errorMessage}` },
				invalidMultiple: {
					object: { mosaicIds: [mosaicIds[0], '12345', mosaicIds[1]] },
					error: `element in array mosaicIds ${errorMessage}`
				}
			},
			dbApiName: 'mosaicsByIds',
			type: 'mosaicDescriptor'
		});
	});

	describe('get mosaic names', () => {
		const endpointUnderTest = '/mosaic/names';

		const createNamespaceWithAlias = (level0, level1, level2, mosaicId, depth) => ({
			namespace: {
				depth,
				level0: convertToLong(level0),
				level1: convertToLong(level1),
				level2: convertToLong(level2),
				alias: {
					type: catapult.model.namespace.aliasType.mosaic,
					mosaicId: convertToLong(mosaicId)
				}
			}
		});
		const createRegisterNamespaceTransaction = (namespaceId, height, index, name) => ({
			meta: {
				height: convertToLong(height),
				index: convertToLong(index)
			},
			transaction: {
				type: catapult.model.EntityType.registerNamespace,
				namespaceId: convertToLong(namespaceId),
				name: { value: () => name }
			}
		});

		const activeNamespacesFromMosaicsIdsFake = sinon.fake(mosaicIds => {
			const namespaces = [
				createNamespaceWithAlias(12345, 0, 0, [0xB6653DE4, 0x78A4895C], 1),
				createNamespaceWithAlias(67891, 38467, 0, [0xB6653DE4, 0x78A4895C], 2),
				createNamespaceWithAlias(23456, 89876, 33437, [0xB6653DE4, 0x78A4895C], 3),
				createNamespaceWithAlias(44651, 78912, 0, [0x45468988, 0x56AB67FF], 2),
				createNamespaceWithAlias(44651, 34567, 0, [0xABD23C14, 0x8F90BC54], 2),
				createNamespaceWithAlias(32175, 0, 0, [0x67AA43F2, 0xD4579B4C], 1)
			];
			return Promise.resolve(namespaces
				.filter(n => (mosaicIds.some(m => convertToLong(m).equals(n.namespace.alias.mosaicId)))));
		});
		const registerNamespaceTransactionsFromNamespaceIdsFake = sinon.fake(namespaceIds => {
			const transactions = [
				createRegisterNamespaceTransaction(12345, 1, 1, 'a1'),
				createRegisterNamespaceTransaction(67891, 1, 1, 'a2'),
				createRegisterNamespaceTransaction(38467, 1, 1, 'b2'),
				createRegisterNamespaceTransaction(23456, 1, 1, 'a3'),
				createRegisterNamespaceTransaction(89876, 1, 1, 'b3'),
				createRegisterNamespaceTransaction(33437, 1, 1, 'c3'),
				createRegisterNamespaceTransaction(44651, 1, 1, 'cat'),
				createRegisterNamespaceTransaction(78912, 1, 1, 'harvest'),
				createRegisterNamespaceTransaction(34567, 1, 1, 'custom'),
				createRegisterNamespaceTransaction(32175, 2, 1, 'first_change'),
				createRegisterNamespaceTransaction(32175, 5, 1, 'second_change'),
				createRegisterNamespaceTransaction(32175, 10, 1, 'third_change'),
				createRegisterNamespaceTransaction(32175, 10, 2, 'fourth_change'),
				createRegisterNamespaceTransaction(32175, 10, 3, 'last_change')
			];

			const requestedTransactions = [];
			namespaceIds.forEach(n => transactions
				.filter(t => n.equals(t.transaction.namespaceId))
				.forEach(t => requestedTransactions.push(t)));

			return Promise.resolve(requestedTransactions);
		});

		const routes = {};
		const server = {
			get: (path, handler) => { routes[path] = handler; },
			post: (path, handler) => { routes[path] = handler; }
		};

		mosaicRoutes.register(server, {
			activeNamespacesByMosaicsIds: activeNamespacesFromMosaicsIdsFake,
			registerNamespaceTransactionsByNamespaceIds: registerNamespaceTransactionsFromNamespaceIdsFake
		});

		let sentResponse;
		const next = () => {};
		const res = {
			send: response => {
				sentResponse = response;
			},
			redirect: () => {
				next();
			}
		};

		beforeEach(() => activeNamespacesFromMosaicsIdsFake.resetHistory());
		beforeEach(() => registerNamespaceTransactionsFromNamespaceIdsFake.resetHistory());

		it('returns alias names from provided mosaicIds, one mosaicId', () => {
			// Arrange:
			const req = { params: { mosaicIds: ['78A4895CB6653DE4'] } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(activeNamespacesFromMosaicsIdsFake.calledOnceWith([[0xB6653DE4, 0x78A4895C]])).to.equal(true);

				expect(registerNamespaceTransactionsFromNamespaceIdsFake.calledOnceWith([
					convertToLong(12345),
					convertToLong(67891),
					convertToLong(38467),
					convertToLong(23456),
					convertToLong(89876),
					convertToLong(33437)
				])).to.equal(true);

				expect(sentResponse).to.deep.equal({
					payload: [
						{
							mosaicId: [0xB6653DE4, 0x78A4895C],
							names: ['a1', 'a2.b2', 'a3.b3.c3']
						}
					],
					type: 'mosaicNamesTuples'
				});
			});
		});

		it('returns alias names from provided mosaicIds, multiple mosaicIds', () => {
			// Arrange:
			const req = { params: { mosaicIds: ['78A4895CB6653DE4', '56AB67FF45468988', '1212121212121212', '8F90BC54ABD23C14'] } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(activeNamespacesFromMosaicsIdsFake.calledOnceWith([
					[0xB6653DE4, 0x78A4895C],
					[0x45468988, 0x56AB67FF],
					[0x12121212, 0x12121212],
					[0xABD23C14, 0x8F90BC54]
				])).to.equal(true);

				expect(registerNamespaceTransactionsFromNamespaceIdsFake.calledOnceWith([
					convertToLong(12345),
					convertToLong(67891),
					convertToLong(38467),
					convertToLong(23456),
					convertToLong(89876),
					convertToLong(33437),
					convertToLong(44651),
					convertToLong(78912),
					convertToLong(44651),
					convertToLong(34567)
				])).to.equal(true);

				expect(sentResponse).to.deep.equal({
					payload: [
						{
							mosaicId: [0xB6653DE4, 0x78A4895C],
							names: ['a1', 'a2.b2', 'a3.b3.c3']
						},
						{
							mosaicId: [0x45468988, 0x56AB67FF],
							names: ['cat.harvest']
						},
						{
							mosaicId: [0x12121212, 0x12121212],
							names: []
						},
						{
							mosaicId: [0xABD23C14, 0x8F90BC54],
							names: ['cat.custom']
						}
					],
					type: 'mosaicNamesTuples'
				});
			});
		});

		it('returns correct alias when namespace is registered several times', () => {
			// Arrange:
			const req = { params: { mosaicIds: ['D4579B4C67AA43F2'] } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(activeNamespacesFromMosaicsIdsFake.calledOnceWith([[0x67AA43F2, 0xD4579B4C]])).to.equal(true);

				expect(registerNamespaceTransactionsFromNamespaceIdsFake.calledOnceWith([
					convertToLong(32175)
				])).to.equal(true);

				expect(sentResponse).to.deep.equal({
					payload: [
						{
							mosaicId: [0x67AA43F2, 0xD4579B4C],
							names: ['last_change']
						}
					],
					type: 'mosaicNamesTuples'
				});
			});
		});

		it('returns empty names list for mosaic with no alias', () => {
			// Arrange:
			const req = { params: { mosaicIds: ['1212121212121212'] } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(activeNamespacesFromMosaicsIdsFake.calledOnceWith([[0x12121212, 0x12121212]])).to.equal(true);

				expect(registerNamespaceTransactionsFromNamespaceIdsFake.calledOnceWith([])).to.equal(true);

				expect(sentResponse).to.deep.equal({
					payload: [
						{
							mosaicId: [0x12121212, 0x12121212],
							names: []
						}
					],
					type: 'mosaicNamesTuples'
				});
			});
		});

		it('returns empty if no mosaic ids are provided', () => {
			// Arrange:
			const req = { params: { mosaicIds: [] } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(activeNamespacesFromMosaicsIdsFake.calledOnceWith([])).to.equal(true);

				expect(registerNamespaceTransactionsFromNamespaceIdsFake.calledOnceWith([])).to.equal(true);

				expect(sentResponse).to.deep.equal({
					payload: [],
					type: 'mosaicNamesTuples'
				});
			});
		});

		it('returns 409 if provided mosaic id is invalid', () => {
			// Arrange:
			const req = { params: { mosaicIds: ['78A4895CB6653DE4', '123XXX', '56AB67FF45468988'] } };

			// Act:
			const route = routes[endpointUnderTest];
			const apiResponse = expect(() => route(req, res, next).then(() => {})).to;

			// Assert:
			apiResponse.throw('element in array mosaicIds has an invalid format');
			apiResponse.with.property('statusCode', 409);
			apiResponse.with.property('message', 'element in array mosaicIds has an invalid format');
			expect(activeNamespacesFromMosaicsIdsFake.notCalled).to.equal(true);
			expect(registerNamespaceTransactionsFromNamespaceIdsFake.notCalled).to.equal(true);
		});
	});
});
