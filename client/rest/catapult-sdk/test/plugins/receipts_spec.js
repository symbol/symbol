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

const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const { expect } = require('chai');

const receiptsPlugin = require('../../src/plugins/receipts');

describe('receipts plugin', () => {
	describe('register schema', () => {
		it('adds receipts system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			receiptsPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 3);
			expect(modelSchema).to.contain.all.keys([
				'receipts.balanceChange',
				'receipts.balanceTransfer',
				'receipts.artifactExpiry'
			]);

			// - receipts.balanceChange
			expect(Object.keys(modelSchema['receipts.balanceChange']).length).to.equal(3);
			expect(modelSchema['receipts.balanceChange']).to.contain.all.keys(['account', 'mosaicId', 'amount']);

			// - receipts.balanceTransfer
			expect(Object.keys(modelSchema['receipts.balanceTransfer']).length).to.equal(4);
			expect(modelSchema['receipts.balanceTransfer']).to.contain.all.keys([
				'sender',
				'recipient',
				'mosaicId',
				'amount'
			]);

			// - receipts.artifactExpiry
			expect(Object.keys(modelSchema['receipts.artifactExpiry']).length).to.equal(1);
			expect(modelSchema['receipts.artifactExpiry']).to.contain.all.keys(['artifactId']);
		});
	});
});
