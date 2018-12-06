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

/**
 * Creates an accountProperties plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const accountPropertiesPlugin = {
	registerSchema: builder => {
		const modificationTypeSchema = {
			propertyType: ModelType.uint64,
			modifications: { type: ModelType.array, schemaName: 'accountProperties.modificationType' }
		};
		builder.addTransactionSupport(EntityType.accountPropertiesAddress, modificationTypeSchema);
		builder.addTransactionSupport(EntityType.accountPropertiesMosaic, modificationTypeSchema);
		builder.addTransactionSupport(EntityType.accountPropertiesEntityType, modificationTypeSchema);
		builder.addSchema('accountProperties.modificationType', {
			modificationType: ModelType.uint64,
			value: ModelType.binary
		});

		builder.addSchema('accountProperties.accountProperties', {
			address: ModelType.binary,
			properties: { type: ModelType.array, schemaName: 'accountProperties.accountProperty' }
		});
		builder.addSchema('accountProperties.accountProperty', {
			propertyType: ModelType.uint64,
			values: ModelType.binary
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
