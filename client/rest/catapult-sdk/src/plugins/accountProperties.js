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

/** @module plugins/accountProperties */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

const propertyTypeBlockOffset = 128;
const PropertyTypeFlags = Object.freeze({
	address: 1,
	mosaic: 2,
	entityType: 4
});

const accountPropertiesCreateBaseCodec = valueCodec => ({
	deserialize: parser => {
		const transaction = {};
		transaction.propertyType = parser.uint8();
		transaction.modifications = [];
		const propertiesCount = parser.uint8();
		for (let i = 0; i < propertiesCount; ++i) {
			transaction.modifications.push({
				modificationType: parser.uint8(),
				value: valueCodec.deserializeValue(parser)
			});
		}
		return transaction;
	},
	serialize: (transaction, serializer) => {
		serializer.writeUint8(transaction.propertyType);
		serializer.writeUint8(transaction.modifications.length);
		for (let i = 0; i < transaction.modifications.length; ++i) {
			serializer.writeUint8(transaction.modifications[i].modificationType);
			valueCodec.serializeValue(serializer, transaction.modifications[i].value);
		}
	}
});

const propertyTypeDescriptors = [
	{
		entityType: EntityType.accountPropertiesAddress,
		schemaPrefix: 'address',
		valueType: ModelType.binary,
		flag: PropertyTypeFlags.address
	},
	{
		entityType: EntityType.accountPropertiesMosaic,
		schemaPrefix: 'mosaic',
		valueType: ModelType.uint64,
		flag: PropertyTypeFlags.mosaic
	},
	{
		entityType: EntityType.accountPropertiesEntityType,
		schemaPrefix: 'entityType',
		valueType: ModelType.uint16,
		flag: PropertyTypeFlags.entityType
	}
];

/**
 * Creates an accountProperties plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const accountPropertiesPlugin = {
	PropertyType: Object.freeze({
		addressAllow: PropertyTypeFlags.address,
		addressBlock: PropertyTypeFlags.address + propertyTypeBlockOffset,
		mosaicAllow: PropertyTypeFlags.mosaic,
		mosaicBlock: PropertyTypeFlags.mosaic + propertyTypeBlockOffset,
		entityTypeAllow: PropertyTypeFlags.entityType,
		entityTypeBlock: PropertyTypeFlags.entityType + propertyTypeBlockOffset
	}),

	registerSchema: builder => {
		const modificationTypeSchema = modificationsSchemaName => ({
			modifications: { type: ModelType.array, schemaName: modificationsSchemaName }
		});
		propertyTypeDescriptors.forEach(propertyTypeDescriptor => {
			// transaction schemas
			builder.addTransactionSupport(
				propertyTypeDescriptor.entityType,
				modificationTypeSchema(`accountProperties.${propertyTypeDescriptor.schemaPrefix}ModificationType`)
			);
			builder.addSchema(`accountProperties.${propertyTypeDescriptor.schemaPrefix}ModificationType`, {
				value: propertyTypeDescriptor.valueType
			});

			// aggregated account property schemas
			builder.addSchema(`accountProperties.${propertyTypeDescriptor.schemaPrefix}AccountProperty`, {
				values: { type: ModelType.array, schemaName: propertyTypeDescriptor.valueType }
			});
		});

		// aggregated account property schemas
		builder.addSchema('accountProperties', {
			accountProperties: { type: ModelType.object, schemaName: 'accountProperties.accountProperties' }
		});
		builder.addSchema('accountProperties.accountProperties', {
			address: ModelType.binary,
			properties: {
				type: ModelType.array,
				schemaName: entity => {
					for (let i = 0; i < propertyTypeDescriptors.length; i++) {
						if ((entity.propertyType & 0x7F) === propertyTypeDescriptors[i].flag)
							return `accountProperties.${propertyTypeDescriptors[i].schemaPrefix}AccountProperty`;
					}
				}
			}
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(
			EntityType.accountPropertiesAddress,
			accountPropertiesCreateBaseCodec({
				deserializeValue: parser => parser.buffer(constants.sizes.addressDecoded),
				serializeValue: (serializer, value) => serializer.writeBuffer(value)
			})
		);

		codecBuilder.addTransactionSupport(
			EntityType.accountPropertiesMosaic,
			accountPropertiesCreateBaseCodec({
				deserializeValue: parser => parser.uint64(),
				serializeValue: (serializer, value) => serializer.writeUint64(value)
			})
		);

		codecBuilder.addTransactionSupport(
			EntityType.accountPropertiesEntityType,
			accountPropertiesCreateBaseCodec({
				deserializeValue: parser => parser.uint16(),
				serializeValue: (serializer, value) => serializer.writeUint16(value)
			})
		);
	}
};

module.exports = accountPropertiesPlugin;
