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

/** @module plugins/receipts */
const ModelType = require('../model/ModelType');

// types 2 (balanceCredit), and 3 (balanceDebit) share the schema `receipts.balanceChange`
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
			source: { type: ModelType.object, schemaName: 'receipts.source' },
			receipts: { type: ModelType.array, schemaName: entity => getBasicReceiptType(entity.type) }
		});

		// addressResolution statements
		builder.addSchema('receipts.entry.address', {
			source: { type: ModelType.object, schemaName: 'receipts.source' },
			resolved: ModelType.binary
		});

		// mosaicResolution statements
		builder.addSchema('receipts.entry.mosaic', {
			source: { type: ModelType.object, schemaName: 'receipts.source' },
			resolved: ModelType.uint64HexIdentifier
		});

		// transaction statements
		builder.addSchema('receipts.balanceChange', {
			version: ModelType.int,
			type: ModelType.int,
			targetAddress: ModelType.binary,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.balanceTransfer', {
			version: ModelType.int,
			type: ModelType.int,
			senderAddress: ModelType.binary,
			recipientAddress: ModelType.binary,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.artifactExpiry', {
			version: ModelType.int,
			type: ModelType.int,
			artifactId: ModelType.uint64HexIdentifier
		});

		builder.addSchema('receipts.inflation', {
			version: ModelType.int,
			type: ModelType.int,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.unknown', {
			version: ModelType.int,
			type: ModelType.int
		});

		// receipts source schema
		builder.addSchema('receipts.source', {
			primaryId: ModelType.int,
			secondaryId: ModelType.int
		});
	},

	registerCodecs: () => {}
};

module.exports = receiptsPlugin;
