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

/** @module plugins/metadata */
import EntityType from '../model/EntityType.js';
import ModelType from '../model/ModelType.js';

/**
 * Creates a metadata plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.accountMetadata, {
			targetAddress: ModelType.encodedAddress,
			scopedMetadataKey: ModelType.uint64HexIdentifier,
			valueSizeDelta: ModelType.int,
			valueSize: ModelType.uint16,
			value: ModelType.binary
		});

		builder.addTransactionSupport(EntityType.mosaicMetadata, {
			targetAddress: ModelType.encodedAddress,
			scopedMetadataKey: ModelType.uint64HexIdentifier,
			targetMosaicId: ModelType.uint64HexIdentifier,
			valueSizeDelta: ModelType.int,
			valueSize: ModelType.uint16,
			value: ModelType.binary
		});

		builder.addTransactionSupport(EntityType.namespaceMetadata, {
			targetAddress: ModelType.encodedAddress,
			scopedMetadataKey: ModelType.uint64HexIdentifier,
			targetNamespaceId: ModelType.uint64HexIdentifier,
			valueSizeDelta: ModelType.int,
			valueSize: ModelType.uint16,
			value: ModelType.binary
		});

		builder.addSchema('metadata', {
			id: ModelType.objectId,
			metadataEntry: { type: ModelType.object, schemaName: 'metadataEntry' }
		});

		builder.addSchema('metadataEntry', {
			version: ModelType.uint16,
			compositeHash: ModelType.binary,
			sourceAddress: ModelType.encodedAddress,
			targetAddress: ModelType.encodedAddress,
			scopedMetadataKey: ModelType.uint64HexIdentifier,
			targetId: ModelType.uint64HexIdentifier,
			metadataType: ModelType.int,
			valueSize: ModelType.uint16,
			value: ModelType.binary
		});
	}
};
