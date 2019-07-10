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

/** @module plugins/accountRestrictions */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

const accountRestrictionTypeBlockOffset = 128;
const AccountRestrictionTypeFlags = Object.freeze({
	address: 1,
	mosaic: 2,
	operation: 4
});

const accountRestrictionsCreateBaseCodec = valueCodec => ({
	deserialize: parser => {
		const transaction = {};
		transaction.restrictionType = parser.uint8();
		transaction.modifications = [];
		const restrictionsCount = parser.uint8();
		for (let i = 0; i < restrictionsCount; ++i) {
			transaction.modifications.push({
				restrictionType: parser.uint8(),
				value: valueCodec.deserializeValue(parser)
			});
		}
		return transaction;
	},
	serialize: (transaction, serializer) => {
		serializer.writeUint8(transaction.restrictionType);
		serializer.writeUint8(transaction.modifications.length);
		for (let i = 0; i < transaction.modifications.length; ++i) {
			serializer.writeUint8(transaction.modifications[i].restrictionType);
			valueCodec.serializeValue(serializer, transaction.modifications[i].value);
		}
	}
});

const accountRestrictionTypeDescriptors = [
	{
		entityType: EntityType.accountRestrictionAddress,
		schemaPrefix: 'address',
		valueType: ModelType.binary,
		flag: AccountRestrictionTypeFlags.address
	},
	{
		entityType: EntityType.accountRestrictionMosaic,
		schemaPrefix: 'mosaic',
		valueType: ModelType.uint64,
		flag: AccountRestrictionTypeFlags.mosaic
	},
	{
		entityType: EntityType.accountRestrictionOperation,
		schemaPrefix: 'operation',
		valueType: ModelType.uint16,
		flag: AccountRestrictionTypeFlags.operation
	}
];

/**
 * Creates an account restrictions plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const accountRestrictionsPlugin = {
	AccountRestrictionType: Object.freeze({
		addressAllow: AccountRestrictionTypeFlags.address,
		addressBlock: AccountRestrictionTypeFlags.address + accountRestrictionTypeBlockOffset,
		mosaicAllow: AccountRestrictionTypeFlags.mosaic,
		mosaicBlock: AccountRestrictionTypeFlags.mosaic + accountRestrictionTypeBlockOffset,
		operationAllow: AccountRestrictionTypeFlags.operation,
		operationBlock: AccountRestrictionTypeFlags.operation + accountRestrictionTypeBlockOffset
	}),

	registerSchema: builder => {
		const modificationTypeSchema = modificationsSchemaName => ({
			modifications: { type: ModelType.array, schemaName: modificationsSchemaName }
		});
		accountRestrictionTypeDescriptors.forEach(restrictionTypeDescriptor => {
			// transaction schemas
			builder.addTransactionSupport(
				restrictionTypeDescriptor.entityType,
				modificationTypeSchema(`accountRestriction.${restrictionTypeDescriptor.schemaPrefix}ModificationType`)
			);
			builder.addSchema(`accountRestriction.${restrictionTypeDescriptor.schemaPrefix}ModificationType`, {
				value: restrictionTypeDescriptor.valueType
			});

			// aggregated account restriction schemas
			builder.addSchema(`accountRestriction.${restrictionTypeDescriptor.schemaPrefix}AccountRestriction`, {
				values: { type: ModelType.array, schemaName: restrictionTypeDescriptor.valueType }
			});
		});

		// aggregated account restrictions schemas
		builder.addSchema('accountRestrictions', {
			accountRestrictions: { type: ModelType.object, schemaName: 'accountRestriction.restrictions' }
		});
		builder.addSchema('accountRestriction.restrictions', {
			address: ModelType.binary,
			restrictions: {
				type: ModelType.array,
				schemaName: entity => {
					for (let i = 0; i < accountRestrictionTypeDescriptors.length; i++) {
						if ((entity.restrictionType & 0x7F) === accountRestrictionTypeDescriptors[i].flag)
							return `accountRestriction.${accountRestrictionTypeDescriptors[i].schemaPrefix}AccountRestriction`;
					}
					return undefined;
				}
			}
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(
			EntityType.accountRestrictionAddress,
			accountRestrictionsCreateBaseCodec({
				deserializeValue: parser => parser.buffer(constants.sizes.addressDecoded),
				serializeValue: (serializer, value) => serializer.writeBuffer(value)
			})
		);

		codecBuilder.addTransactionSupport(
			EntityType.accountRestrictionMosaic,
			accountRestrictionsCreateBaseCodec({
				deserializeValue: parser => parser.uint64(),
				serializeValue: (serializer, value) => serializer.writeUint64(value)
			})
		);

		codecBuilder.addTransactionSupport(
			EntityType.accountRestrictionOperation,
			accountRestrictionsCreateBaseCodec({
				deserializeValue: parser => parser.uint16(),
				serializeValue: (serializer, value) => serializer.writeUint16(value)
			})
		);
	}
};

module.exports = accountRestrictionsPlugin;
