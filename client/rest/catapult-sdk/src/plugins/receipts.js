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

/**
 * Creates a receipts plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const receiptsPlugin = {
	registerSchema: builder => {
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
	}
};

module.exports = receiptsPlugin;
