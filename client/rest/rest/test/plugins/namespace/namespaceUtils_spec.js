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

const { convertToLong } = require('../../../src/db/dbUtils');
const namespaceUtils = require('../../../src/plugins/namespace/namespaceUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { aliasNamesRoutesProcessor } = namespaceUtils;

describe('namespace utils', () => {
	describe('aliasNamesRoutesProcessor', () => {
		const createNamespace = (level0, level1, level2, depth) => ({
			namespace: {
				depth,
				level0: convertToLong(level0),
				level1: convertToLong(level1),
				level2: convertToLong(level2)
			}
		});
		const activeNamespacesWithAliasFake = sinon.fake(() => {
			const namespaces = [
				createNamespace(12345, 0, 0, 1),
				createNamespace(67891, 38467, 0, 2),
				createNamespace(23456, 89876, 33437, 3),
				createNamespace(88448, 10001, 99999, 1)
			];
			return Promise.resolve(namespaces);
		});

		const createRegisterNamespaceTransaction = (namespaceId, height, index, name) => ({
			meta: {
				height: convertToLong(height),
				index
			},
			transaction: {
				type: catapult.model.EntityType.registerNamespace,
				id: convertToLong(namespaceId),
				name: { value: () => name }
			}
		});
		const registerNamespaceTransactionsFromNamespaceIdsFake = sinon.fake(() => {
			const transactions = [
				createRegisterNamespaceTransaction(12345, 1, 1, 'a'),
				createRegisterNamespaceTransaction(67891, 1, 1, 'b'),
				createRegisterNamespaceTransaction(38467, 1, 1, 'c'),
				createRegisterNamespaceTransaction(23456, 1, 1, 'd'),
				createRegisterNamespaceTransaction(89876, 1, 1, 'e'),
				createRegisterNamespaceTransaction(33437, 1, 1, 'f'),
				createRegisterNamespaceTransaction(33437, 2, 2, 'g'),
				createRegisterNamespaceTransaction(33437, 2, 1, 'h')
			];
			return Promise.resolve(transactions);
		});

		const sendFake = sinon.fake();
		const nextFake = sinon.fake();

		const db = {
			activeNamespacesWithAlias: activeNamespacesWithAliasFake,
			registerNamespaceTransactionsByNamespaceIds: registerNamespaceTransactionsFromNamespaceIdsFake
		};

		const aliasType = 1;
		const getParamsFake = sinon.fake(() => [1, 2]);
		const namespaceFilterFake = sinon.fake(() => true);
		const fieldName = 'testAliasFieldName';
		const schemaName = 'testSchemaName';
		const processorFunction = aliasNamesRoutesProcessor(
			db,
			aliasType,
			getParamsFake,
			namespaceFilterFake,
			fieldName,
			schemaName
		);

		beforeEach(() => {
			sendFake.resetHistory();
			nextFake.resetHistory();
			activeNamespacesWithAliasFake.resetHistory();
			registerNamespaceTransactionsFromNamespaceIdsFake.resetHistory();
			getParamsFake.resetHistory();
			namespaceFilterFake.resetHistory();
		});

		it('calls activeNamespacesWithAlias', () => {
			// Arrange:
			const req = {};

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(activeNamespacesWithAliasFake.calledOnce).to.equal(true);

				expect(activeNamespacesWithAliasFake.firstCall.args[0]).to.equal(aliasType);

				expect(activeNamespacesWithAliasFake.firstCall.args[1]).to.deep.equal([1, 2]);
			});
		});

		it('calls registerNamespaceTransactionsByNamespaceIds', () => {
			// Arrange:
			const req = {};

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(registerNamespaceTransactionsFromNamespaceIdsFake.calledOnce).to.equal(true);

				expect(registerNamespaceTransactionsFromNamespaceIdsFake.firstCall.args[0])
					.to.deep.equal([12345, 67891, 38467, 23456, 89876, 33437, 88448].map(convertToLong));
			});
		});

		it('calls get params', () => {
			// Arrange:
			const req = { params: { ids: [1, 2, 3] } };

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(getParamsFake.calledOnce).to.equal(true);

				expect(getParamsFake.firstCall.args[0]).to.deep.equal(req);
			});
		});

		it('returns alias names', () => {
			// Arrange:
			const req = {};

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(sendFake.firstCall.args[0]).to.deep.equal({
					payload: {
						testSchemaName: [
							{
								[fieldName]: 1,
								names: [
									'a',
									'b.c',
									'd.e.g'
								]
							},
							{
								[fieldName]: 2,
								names: [
									'a',
									'b.c',
									'd.e.g'
								]
							}
						]
					},
					type: schemaName
				});

				expect(nextFake.calledOnce).to.equal(true);
			});
		});
	});
});
