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

const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const ModelType = require('../../src/model/ModelType');
const receiptsPlugin = require('../../src/plugins/receipts');
const schemaFormatter = require('../../src/utils/schemaFormatter');
const { expect } = require('chai');

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
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 14);
			expect(modelSchema).to.contain.all.keys([
				'addressResolutionStatement',
				'addressResolutionStatement.statement',
				'mosaicResolutionStatement',
				'mosaicResolutionStatement.statement',
				'transactionStatement',
				'transactionStatement.statement',
				'receipts.balanceChange',
				'receipts.balanceTransfer',
				'receipts.artifactExpiry',
				'receipts.inflation',
				'receipts.entry.address',
				'receipts.entry.mosaic',
				'receipts.unknown',
				'receipts.source'
			]);

			// - addressResolutionStatement
			// - mosaicResolutionStatement
			// - transactionStatement
			['addressResolution', 'mosaicResolution', 'transaction'].forEach(statementType => {
				expect(Object.keys(modelSchema[`${statementType}Statement`]).length).to.equal(2);
				expect(modelSchema[`${statementType}Statement`]).to.contain.all.keys(['statement']);
			});

			// - addressResolutionStatement.statement
			expect(Object.keys(modelSchema['addressResolutionStatement.statement']).length).to.equal(3);
			expect(modelSchema['addressResolutionStatement.statement']).to.contain.all.keys([
				'height',
				'unresolved',
				'resolutionEntries'
			]);

			// - mosaicResolutionStatement
			expect(Object.keys(modelSchema['mosaicResolutionStatement.statement']).length).to.equal(3);
			expect(modelSchema['mosaicResolutionStatement.statement']).to.contain.all.keys([
				'height',
				'unresolved',
				'resolutionEntries'
			]);

			// - transactionStatement
			expect(Object.keys(modelSchema['transactionStatement.statement']).length).to.equal(3);
			expect(modelSchema['transactionStatement.statement']).to.contain.all.keys([
				'height', 'source', 'receipts'
			]);

			// - receipts.entry.address
			expect(Object.keys(modelSchema['receipts.entry.address']).length).to.equal(2);
			expect(modelSchema['receipts.entry.address']).to.contain.all.keys([
				'source', 'resolved'
			]);

			// - receipts.entry.mosaic
			expect(Object.keys(modelSchema['receipts.entry.mosaic']).length).to.equal(2);
			expect(modelSchema['receipts.entry.mosaic']).to.contain.all.keys([
				'source', 'resolved'
			]);

			// - receipts.balanceChange
			expect(Object.keys(modelSchema['receipts.balanceChange']).length).to.equal(5);
			expect(modelSchema['receipts.balanceChange']).to.contain.all.keys([
				'version', 'type', 'targetAddress', 'mosaicId', 'amount'
			]);

			// - receipts.balanceTransfer
			expect(Object.keys(modelSchema['receipts.balanceTransfer']).length).to.equal(6);
			expect(modelSchema['receipts.balanceTransfer']).to.contain.all.keys([
				'version', 'type', 'senderAddress', 'recipientAddress', 'mosaicId', 'amount'
			]);

			// - receipts.artifactExpiry
			expect(Object.keys(modelSchema['receipts.artifactExpiry']).length).to.equal(3);
			expect(modelSchema['receipts.artifactExpiry']).to.contain.all.keys([
				'version', 'type', 'artifactId'
			]);

			// - receipts.inflation
			expect(Object.keys(modelSchema['receipts.inflation']).length).to.equal(4);
			expect(modelSchema['receipts.inflation']).to.contain.all.keys([
				'version', 'type', 'mosaicId', 'amount'
			]);

			// - receipts.unknown
			expect(Object.keys(modelSchema['receipts.unknown']).length).to.equal(2);
			expect(modelSchema['receipts.unknown']).to.contain.all.keys([
				'version', 'type'
			]);

			// - receipts.source
			expect(Object.keys(modelSchema['receipts.source']).length).to.equal(2);
			expect(modelSchema['receipts.source']).to.contain.all.keys([
				'primaryId', 'secondaryId'
			]);
		});
	});

	describe('conditional schema', () => {
		describe('uses the correct conditional schema depending on receipt type', () => {
			const formatReceipt = receipt => {
				// Arrange:
				const formattingRules = {
					[ModelType.none]: () => 'none',
					[ModelType.binary]: () => 'binary',
					[ModelType.uint64]: () => 'uint64',
					[ModelType.uint64HexIdentifier]: () => 'uint64HexIdentifier',
					[ModelType.objectId]: () => 'objectId',
					[ModelType.string]: () => 'string',
					[ModelType.int]: () => 'int'
				};
				const transactionStatement = {
					statement: {
						height: null,
						source: { primaryId: null, secondaryId: null },
						receipts: [receipt]
					}
				};
				const builder = new ModelSchemaBuilder();

				// Act:
				receiptsPlugin.registerSchema(builder);
				const modelSchema = builder.build();
				const unwrappedFormattedEntity = schemaFormatter.format(
					transactionStatement,
					modelSchema.transactionStatement,
					modelSchema,
					formattingRules
				).statement;

				// Assert
				expect(Object.keys(unwrappedFormattedEntity).length).to.equal(3);
				expect(unwrappedFormattedEntity).to.contain.all.keys(['height', 'source', 'receipts']);
				expect(unwrappedFormattedEntity.receipts.length).to.equal(1);
				return unwrappedFormattedEntity.receipts[0];
			};

			it('formats balance transfer receipt type', () => {
				// Arrange:
				const balanceTransferReceipt = {
					version: 1,
					type: 0x1000,
					senderAddress: null,
					recipientAddress: null,
					mosaicId: null,
					amount: null
				};

				// Act:
				const formattedReceipt = formatReceipt(balanceTransferReceipt);

				// Assert:
				expect(formattedReceipt).to.contain.all.keys([
					'version',
					'type',
					'senderAddress',
					'recipientAddress',
					'mosaicId',
					'amount'
				]);
			});

			it('formats balance change receipt type', () => {
				// Arrange:
				const balanceChangeReceipt = {
					version: 1,
					type: 0x2000,
					targetAddress: null,
					mosaicId: null,
					amount: null
				};

				// Act:
				const formattedReceipt = formatReceipt(balanceChangeReceipt);

				// Assert:
				expect(formattedReceipt).to.contain.all.keys([
					'version',
					'type',
					'targetAddress',
					'mosaicId',
					'amount'
				]);
			});

			it('formats artifact expiry receipt type', () => {
				// Arrange:
				const artifactExpiryReceipt = {
					version: 1,
					type: 0x4000,
					artifactId: null
				};

				// Act:
				const formattedReceipt = formatReceipt(artifactExpiryReceipt);

				// Assert:
				expect(formattedReceipt).to.contain.all.keys([
					'version',
					'type',
					'artifactId'
				]);
			});

			it('formats inflation receipt type', () => {
				// Arrange:
				const inflationReceipt = {
					version: 1,
					type: 0x5000,
					mosaicId: null,
					amount: null
				};

				// Act:
				const formattedReceipt = formatReceipt(inflationReceipt);

				// Assert:
				expect(formattedReceipt).to.contain.all.keys([
					'version',
					'type',
					'mosaicId',
					'amount'
				]);
			});

			it('formats unknown receipt type', () => {
				// Arrange:
				const unknownReceipt = {
					version: null,
					type: 82356235,
					unknownProperty1: null,
					unknownProperty2: null
				};

				// Act:
				const formattedReceipt = formatReceipt(unknownReceipt);

				// Assert:
				expect(formattedReceipt).to.contain.all.keys([
					'version',
					'type'
				]);
			});
		});
	});
});
