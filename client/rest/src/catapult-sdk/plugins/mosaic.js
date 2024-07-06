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

/** @module plugins/mosaic */
import ModelType from '../model/ModelType.js';
import { models } from 'symbol-sdk/symbol';

/**
 * Creates a mosaic plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport(models.TransactionType.MOSAIC_DEFINITION, {
			id: ModelType.uint64HexIdentifier,
			duration: ModelType.uint64,
			nonce: ModelType.uint32,
			flags: ModelType.uint8,
			divisibility: ModelType.uint8
		});

		builder.addTransactionSupport(models.TransactionType.MOSAIC_SUPPLY_CHANGE, {
			mosaicId: ModelType.uint64HexIdentifier,
			delta: ModelType.uint64,
			action: ModelType.uint8
		});

		builder.addTransactionSupport(models.TransactionType.MOSAIC_SUPPLY_REVOCATION, {
			sourceAddress: ModelType.encodedAddress,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64
		});

		builder.addSchema('mosaicDescriptor', {
			id: ModelType.objectId,
			mosaic: { type: ModelType.object, schemaName: 'mosaicDescriptor.mosaic' }
		});

		builder.addSchema('mosaicDescriptor.mosaic', {
			version: ModelType.uint16,
			id: ModelType.uint64HexIdentifier,
			supply: ModelType.uint64,
			startHeight: ModelType.uint64,
			ownerAddress: ModelType.encodedAddress,
			revision: ModelType.int,
			flags: ModelType.uint8,
			divisibility: ModelType.uint8,
			duration: ModelType.uint64
		});
	}
};
