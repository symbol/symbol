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

/** @module plugins/receipts */
const ModelType = require('../model/ModelType');

const ReceiptType = {
	1: 'receipts.balanceTransfer',
	2: 'receipts.balanceChange',
	3: 'receipts.balanceChange',
	4: 'receipts.artifactExpiry',
	5: 'receipts.inflation'
};

const getBasicReceiptType = type => ReceiptType[(type & 0xF000) >> 12] || 'receipts.unknown';

/**
 * Creates a receipts plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const receiptsPlugin = {
	registerSchema: builder => {
		const addStatementSchema = (statementType, schema) => {
			const schemaName = `${statementType}Statement`;
			builder.addSchema(schemaName, {
				id: ModelType.objectId,
				statement: { type: ModelType.object, schemaName: `${schemaName}.statement` }
			});
			builder.addSchema(`${schemaName}.statement`, schema);
		};
		addStatementSchema('addressResolution', {
			height: ModelType.uint64,
			unresolved: ModelType.binary,
			resolutionEntries: { type: ModelType.array, schemaName: 'receipts.entry.address' }
		});
		addStatementSchema('mosaicResolution', {
			height: ModelType.uint64,
			unresolved: ModelType.uint64HexIdentifier,
			resolutionEntries: { type: ModelType.array, schemaName: 'receipts.entry.mosaic' }
		});
		addStatementSchema('transaction', {
			height: ModelType.uint64,
			receipts: { type: ModelType.array, schemaName: entity => getBasicReceiptType(entity.type) }
		});

		builder.addSchema('receipts.entry.address', {
			resolved: ModelType.binary
		});

		builder.addSchema('receipts.entry.mosaic', {
			resolved: ModelType.uint64HexIdentifier
		});

		builder.addSchema('receipts.balanceChange', {
			targetAddress: ModelType.binary,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.balanceTransfer', {
			senderAddress: ModelType.binary,
			recipientAddress: ModelType.binary,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.artifactExpiry', {
			artifactId: ModelType.uint64HexIdentifier
		});

		builder.addSchema('receipts.inflation', {
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.unknown', {});
	},

	registerCodecs: () => {}
};

module.exports = receiptsPlugin;
