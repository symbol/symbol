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
	4: 'receipts.artifactExpiry'
};

const getBasicReceiptType = type => ReceiptType[(type & 0xF000) >> 12] || 'receipts.unknown';

/**
 * Creates a receipts plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const receiptsPlugin = {
	registerSchema: builder => {
		builder.addSchema('receipts', {
			transactionStatements: { type: ModelType.array, schemaName: 'receipts.transactionStatement' },
			addressResolutionStatements: { type: ModelType.array, schemaName: 'receipts.addressResolutionStatement' },
			mosaicResolutionStatements: { type: ModelType.array, schemaName: 'receipts.mosaicResolutionStatement' }
		});

		builder.addSchema('receipts.addressResolutionStatement', {
			height: ModelType.uint64,
			unresolved: ModelType.binary,
			entries: { type: ModelType.array, schemaName: 'receipts.entry.address' }
		});

		builder.addSchema('receipts.mosaicResolutionStatement', {
			height: ModelType.uint64,
			unresolved: ModelType.uint64,
			entries: { type: ModelType.array, schemaName: 'receipts.entry.mosaic' }
		});

		builder.addSchema('receipts.transactionStatement', {
			height: ModelType.uint64,
			receipts: { type: ModelType.array, schemaName: entity => getBasicReceiptType(entity.type) }
		});

		builder.addSchema('receipts.entry.address', {
			resolved: ModelType.binary
		});

		builder.addSchema('receipts.entry.mosaic', {
			resolved: ModelType.uint64
		});

		builder.addSchema('receipts.balanceChange', {
			account: ModelType.binary,
			mosaicId: ModelType.uint64,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.balanceTransfer', {
			sender: ModelType.binary,
			recipient: ModelType.binary,
			mosaicId: ModelType.uint64,
			amount: ModelType.uint64
		});

		builder.addSchema('receipts.artifactExpiry', {
			artifactId: ModelType.uint64
		});

		builder.addSchema('receipts.unknown', {});
	},

	registerCodecs: codecBuilder => {}
};

module.exports = receiptsPlugin;
