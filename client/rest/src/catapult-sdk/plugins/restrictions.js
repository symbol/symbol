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
import ModelType from '../model/ModelType.js';
import { models } from 'symbol-sdk/symbol';

// const accountRestrictionTypeOutgoingOffset = 0x4000;
const accountRestrictionTypeBlockOffset = 0x8000;
const AccountRestrictionTypeFlags = Object.freeze({
	address: 0x0001,
	mosaic: 0x0002,
	operation: 0x0004
});

const accountRestrictionTypeDescriptors = [
	{
		entityType: models.TransactionType.ACCOUNT_ADDRESS_RESTRICTION,
		schemaPrefix: 'address',
		valueType: ModelType.binary,
		flag: AccountRestrictionTypeFlags.address
	},
	{
		entityType: models.TransactionType.ACCOUNT_MOSAIC_RESTRICTION,
		schemaPrefix: 'mosaic',
		valueType: ModelType.uint64HexIdentifier,
		flag: AccountRestrictionTypeFlags.mosaic
	},
	{
		entityType: models.TransactionType.ACCOUNT_OPERATION_RESTRICTION,
		schemaPrefix: 'operation',
		valueType: ModelType.uint16,
		flag: AccountRestrictionTypeFlags.operation
	}
];

/**
 * Creates a restrictions plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
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
			version: ModelType.uint16,
			address: ModelType.encodedAddress,
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
		builder.addTransactionSupport(models.TransactionType.MOSAIC_ADDRESS_RESTRICTION, {
			mosaicId: ModelType.uint64HexIdentifier,
			restrictionKey: ModelType.uint64HexIdentifier,
			targetAddress: ModelType.encodedAddress,
			previousRestrictionValue: ModelType.uint64,
			newRestrictionValue: ModelType.uint64
		});

		// MosaicGlobalRestrictionTransaction transaction schema
		builder.addTransactionSupport(models.TransactionType.MOSAIC_GLOBAL_RESTRICTION, {
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
			version: ModelType.uint16,
			compositeHash: ModelType.binary,
			entryType: ModelType.uint32,
			mosaicId: ModelType.uint64HexIdentifier,
			targetAddress: ModelType.encodedAddress,
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
	}
};
