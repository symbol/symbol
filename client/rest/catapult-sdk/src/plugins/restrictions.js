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

/** @module plugins/restrictions */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

// const accountRestrictionTypeOutgoingOffset = 0x4000;
const accountRestrictionTypeBlockOffset = 0x8000;
const AccountRestrictionTypeFlags = Object.freeze({
	address: 0x0001,
	mosaic: 0x0002,
	operation: 0x0004
});

const accountRestrictionsCreateBaseCodec = valueCodec => ({
	deserialize: parser => {
		const transaction = {};
		transaction.restrictionFlags = parser.uint16();
		const restrictionAdditionsCount = parser.uint8();
		const restrictionDeletionsCount = parser.uint8();
		transaction.accountRestrictionTransactionBody_Reserved1 = parser.uint32();

		transaction.restrictionAdditions = [];
		for (let i = 0; i < restrictionAdditionsCount; ++i)
			transaction.restrictionAdditions.push(valueCodec.deserializeValue(parser));

		transaction.restrictionDeletions = [];
		for (let i = 0; i < restrictionDeletionsCount; ++i)
			transaction.restrictionDeletions.push(valueCodec.deserializeValue(parser));

		return transaction;
	},
	serialize: (transaction, serializer) => {
		serializer.writeUint16(transaction.restrictionFlags);
		serializer.writeUint8(transaction.restrictionAdditions.length);
		serializer.writeUint8(transaction.restrictionDeletions.length);
		serializer.writeUint32(transaction.accountRestrictionTransactionBody_Reserved1);
		transaction.restrictionAdditions.forEach(key => {
			valueCodec.serializeValue(serializer, key);
		});
		transaction.restrictionDeletions.forEach(key => {
			valueCodec.serializeValue(serializer, key);
		});
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
		valueType: ModelType.uint64HexIdentifier,
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
 * Creates a restrictions plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const restrictionsPlugin = {
	AccountRestrictionType: Object.freeze({
		addressAllow: AccountRestrictionTypeFlags.address,
		addressBlock: AccountRestrictionTypeFlags.address + accountRestrictionTypeBlockOffset,
		mosaicAllow: AccountRestrictionTypeFlags.mosaic,
		mosaicBlock: AccountRestrictionTypeFlags.mosaic + accountRestrictionTypeBlockOffset,
		operationAllow: AccountRestrictionTypeFlags.operation,
		operationBlock: AccountRestrictionTypeFlags.operation + accountRestrictionTypeBlockOffset
	}),

	registerSchema: builder => {
		/**
		 * Account restrictions scope
		 */
		accountRestrictionTypeDescriptors.forEach(restrictionTypeDescriptor => {
			// transaction schemas
			builder.addTransactionSupport(restrictionTypeDescriptor.entityType, {
				restrictionFlags: ModelType.uint16,
				restrictionAdditions: { type: ModelType.array, schemaName: restrictionTypeDescriptor.valueType },
				restrictionDeletions: { type: ModelType.array, schemaName: restrictionTypeDescriptor.valueType }
			});

			// aggregated account restriction subschemas
			builder.addSchema(`accountRestriction.${restrictionTypeDescriptor.schemaPrefix}AccountRestriction`, {
				restrictionFlags: ModelType.uint16,
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
						if ((entity.restrictionFlags & 0x3FFF) === accountRestrictionTypeDescriptors[i].flag)
							// the following schemas were added in the previous loop
							return `accountRestriction.${accountRestrictionTypeDescriptors[i].schemaPrefix}AccountRestriction`;
					}
					return 'accountRestriction.fallback';
				}
			}
		});
		builder.addSchema('accountRestriction.fallback', {});

		/**
		 * Mosaic restrictions scope
		 */
		// MosaicAddressRestrictionTransaction transaction schema
		builder.addTransactionSupport(EntityType.mosaicRestrictionAddress, {
			mosaicId: ModelType.uint64HexIdentifier,
			restrictionKey: ModelType.uint64HexIdentifier,
			targetAddress: ModelType.binary,
			previousRestrictionValue: ModelType.uint64,
			newRestrictionValue: ModelType.uint64
		});

		// MosaicGlobalRestrictionTransaction transaction schema
		builder.addTransactionSupport(EntityType.mosaicRestrictionGlobal, {
			mosaicId: ModelType.uint64HexIdentifier,
			referenceMosaicId: ModelType.uint64HexIdentifier,
			restrictionKey: ModelType.uint64HexIdentifier,
			previousRestrictionValue: ModelType.uint64,
			newRestrictionValue: ModelType.uint64,
			previousRestrictionType: ModelType.uint8,
			newRestrictionType: ModelType.uint8
		});

		// mosaic restriction schemas
		builder.addSchema('mosaicRestrictions', {
			id: ModelType.objectId,
			mosaicRestrictionEntry: { type: ModelType.object, schemaName: 'mosaicRestrictions.entry' }
		});
		builder.addSchema('mosaicRestrictions.entry', {
			compositeHash: ModelType.binary,
			entryType: ModelType.uint32,
			mosaicId: ModelType.uint64HexIdentifier,
			targetAddress: ModelType.binary,
			restrictions: { type: ModelType.array, schemaName: 'mosaicRestrictions.entry.restrictions' }
		});
		builder.addSchema('mosaicRestrictions.entry.restrictions', {
			key: ModelType.uint64,
			value: ModelType.uint64,
			restriction: { type: ModelType.object, schemaName: 'mosaicRestrictions.entry.restrictions.restriction' }
		});
		builder.addSchema('mosaicRestrictions.entry.restrictions.restriction', {
			referenceMosaicId: ModelType.uint64HexIdentifier,
			restrictionValue: ModelType.uint64,
			restrictionType: ModelType.uint8
		});
	},

	registerCodecs: codecBuilder => {
		// account restrictions address
		codecBuilder.addTransactionSupport(
			EntityType.accountRestrictionAddress,
			accountRestrictionsCreateBaseCodec({
				deserializeValue: parser => parser.buffer(constants.sizes.addressDecoded),
				serializeValue: (serializer, value) => serializer.writeBuffer(value)
			})
		);

		// account restrictions mosaic
		codecBuilder.addTransactionSupport(
			EntityType.accountRestrictionMosaic,
			accountRestrictionsCreateBaseCodec({
				deserializeValue: parser => parser.uint64(),
				serializeValue: (serializer, value) => serializer.writeUint64(value)
			})
		);

		// account restrictions operation
		codecBuilder.addTransactionSupport(
			EntityType.accountRestrictionOperation,
			accountRestrictionsCreateBaseCodec({
				deserializeValue: parser => parser.uint16(),
				serializeValue: (serializer, value) => serializer.writeUint16(value)
			})
		);

		// mosaic restrictions address
		codecBuilder.addTransactionSupport(EntityType.mosaicRestrictionAddress, {
			deserialize: parser => {
				const transaction = {};
				transaction.mosaicId = parser.uint64();
				transaction.restrictionKey = parser.uint64();
				transaction.previousRestrictionValue = parser.uint64();
				transaction.newRestrictionValue = parser.uint64();
				transaction.targetAddress = parser.buffer(constants.sizes.addressDecoded);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint64(transaction.restrictionKey);
				serializer.writeUint64(transaction.previousRestrictionValue);
				serializer.writeUint64(transaction.newRestrictionValue);
				serializer.writeBuffer(transaction.targetAddress);
			}
		});

		// mosaic restrictions global
		codecBuilder.addTransactionSupport(EntityType.mosaicRestrictionGlobal, {
			deserialize: parser => {
				const transaction = {};
				transaction.mosaicId = parser.uint64();
				transaction.referenceMosaicId = parser.uint64();
				transaction.restrictionKey = parser.uint64();
				transaction.previousRestrictionValue = parser.uint64();
				transaction.newRestrictionValue = parser.uint64();
				transaction.previousRestrictionType = parser.uint8();
				transaction.newRestrictionType = parser.uint8();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint64(transaction.referenceMosaicId);
				serializer.writeUint64(transaction.restrictionKey);
				serializer.writeUint64(transaction.previousRestrictionValue);
				serializer.writeUint64(transaction.newRestrictionValue);
				serializer.writeUint8(transaction.previousRestrictionType);
				serializer.writeUint8(transaction.newRestrictionType);
			}
		});
	}
};

module.exports = restrictionsPlugin;
