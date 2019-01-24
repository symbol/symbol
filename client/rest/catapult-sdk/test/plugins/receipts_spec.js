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
const ModelType = require('../../src/model/ModelType');
const schemaFormatter = require('../../src/utils/schemaFormatter');
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
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 9);
			expect(modelSchema).to.contain.all.keys([
				'receipts',
				'receipts.addressResolutionStatement',
				'receipts.mosaicResolutionStatement',
				'receipts.transactionStatement',
				'receipts.balanceChange',
				'receipts.balanceTransfer',
				'receipts.artifactExpiry',
				'receipts.entry.address',
				'receipts.entry.mosaic'
			]);

			// -  receipts
			expect(Object.keys(modelSchema.receipts).length).to.equal(3);
			expect(modelSchema.receipts).to.contain.all.keys([
				'addressResolutionStatements',
				'mosaicResolutionStatements',
				'transactionStatements'
			]);

			// - receipts.addressResolutionStatement
			expect(Object.keys(modelSchema['receipts.addressResolutionStatement']).length).to.equal(3);
			expect(modelSchema['receipts.addressResolutionStatement']).to.contain.all.keys([
				'height',
				'unresolved',
				'entries'
			]);

			// - receipts.mosaicResolutionStatement
			expect(Object.keys(modelSchema['receipts.mosaicResolutionStatement']).length).to.equal(3);
			expect(modelSchema['receipts.mosaicResolutionStatement']).to.contain.all.keys([
				'height',
				'unresolved',
				'entries'
			]);

			// - receipts.transactionStatement
			expect(Object.keys(modelSchema['receipts.transactionStatement']).length).to.equal(2);
			expect(modelSchema['receipts.transactionStatement']).to.contain.all.keys([
				'height',
				'receipts'
			]);

			// - receipts.entry.address
			expect(Object.keys(modelSchema['receipts.entry.address']).length).to.equal(1);
			expect(modelSchema['receipts.entry.address']).to.contain.all.keys([
				'resolved'
			]);

			// - receipts.entry.mosaic
			expect(Object.keys(modelSchema['receipts.entry.mosaic']).length).to.equal(1);
			expect(modelSchema['receipts.entry.mosaic']).to.contain.all.keys([
				'resolved'
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

	describe('conditional schema', () => {
		describe('uses the correct conditional schema depending on receipt type', () => {
			// Arrange:
			const formattingRules = {
				[ModelType.none]: () => 'none',
				[ModelType.binary]: () => 'binary',
				[ModelType.uint64]: () => 'uint64',
				[ModelType.objectId]: () => 'objectId',
				[ModelType.string]: () => 'string'
			};
			const builder = new ModelSchemaBuilder();
			const balanceChangeReceipt = {
				version: 1,
				type: 0x1000,
				account: null,
				mosaicId: null,
				amount: null
			};
			const balanceTransferReceipt = {
				version: 1,
				type: 0x2000,
				sender: null,
				recipient: null,
				mosaicId: null,
				amount: null
			};
			const artifcatExpiryReceipt = {
				version: 1,
				type: 0x3000,
				artifactId: null
			};
			const transactionStatement = {
				height: null,
				source: {
					primaryId: null,
					secondaryId: null
				},
				receipts: [
					balanceChangeReceipt,
					balanceTransferReceipt,
					artifcatExpiryReceipt
				]
			};

			// Act:
			receiptsPlugin.registerSchema(builder);
			const modelSchema = builder.build();
			const formattedEntity = schemaFormatter.format(
				transactionStatement,
				modelSchema['receipts.transactionStatement'],
				modelSchema,
				formattingRules
			);

			// Assert:
			it('creates correct format keys', () => {
				// - formatted entity keys
				expect(Object.keys(formattedEntity).length).to.equal(3);
				expect(formattedEntity).to.contain.all.keys([
					'height',
					'source',
					'receipts'
				]);
			});

			it('formats balance change receipt type', () => {
				// - balance change receipt type
				expect(Object.keys(formattedEntity.receipts[0]).length).to.equal(5);
				expect(formattedEntity.receipts[0]).to.contain.all.keys([
					'version',
					'type',
					'account',
					'mosaicId',
					'amount'
				]);
			});

			it('formats balance transfer receipt type', () => {
				// - balance transfer receipt type
				expect(Object.keys(formattedEntity.receipts[1]).length).to.equal(6);
				expect(formattedEntity.receipts[1]).to.contain.all.keys([
					'version',
					'type',
					'sender',
					'recipient',
					'mosaicId',
					'amount'
				]);
			});

			it('formats artifact expiry receipt type', () => {
				// - artifact expiry receipt type
				expect(Object.keys(formattedEntity.receipts[2]).length).to.equal(3);
				expect(formattedEntity.receipts[2]).to.contain.all.keys([
					'version',
					'type',
					'artifactId'
				]);
			});
		});
	});
});
