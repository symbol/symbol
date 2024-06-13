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

import ModelSchemaBuilder from '../../../src/catapult-sdk/model/ModelSchemaBuilder.js';
import restrictionsPlugin from '../../../src/catapult-sdk/plugins/restrictions.js';
import { expect } from 'chai';

const { AccountRestrictionType } = restrictionsPlugin;

describe('restrictions plugin', () => {
	describe('account restriction types enumeration', () => {
		it('contains valid values', () => {
			const accountRestrictionTypeBlockOffset = 0x8000;

			// Assert:
			expect(AccountRestrictionType.addressAllow).to.equal(0x0001);
			expect(AccountRestrictionType.addressBlock).to.equal(0x0001 + accountRestrictionTypeBlockOffset);
			expect(AccountRestrictionType.mosaicAllow).to.equal(0x0002);
			expect(AccountRestrictionType.mosaicBlock).to.equal(0x0002 + accountRestrictionTypeBlockOffset);
			expect(AccountRestrictionType.operationAllow).to.equal(0x0004);
			expect(AccountRestrictionType.operationBlock).to.equal(0x0004 + accountRestrictionTypeBlockOffset);
		});
	});

	describe('register schema', () => {
		// Arrange:
		const builder = new ModelSchemaBuilder();
		const numDefaultKeys = Object.keys(builder.build()).length;
		const accountRestrictionSchemas = [
			'accountRestriction.addressAccountRestriction',
			'accountRestriction.mosaicAccountRestriction',
			'accountRestriction.operationAccountRestriction'
		];

		// Act:
		restrictionsPlugin.registerSchema(builder);
		const modelSchema = builder.build();

		// Assert:
		it('adds restrictions system schema', () => {
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 15);
			expect(modelSchema).to.contain.all.keys([
				'TransactionType.ACCOUNT_ADDRESS_RESTRICTION',
				'TransactionType.ACCOUNT_MOSAIC_RESTRICTION',
				'TransactionType.ACCOUNT_OPERATION_RESTRICTION',
				'TransactionType.MOSAIC_ADDRESS_RESTRICTION',
				'TransactionType.MOSAIC_GLOBAL_RESTRICTION',
				'accountRestrictions',
				'accountRestriction.restrictions',
				'accountRestriction.fallback',
				'mosaicRestrictions',
				'mosaicRestrictions.entry',
				'mosaicRestrictions.entry.restrictions',
				'mosaicRestrictions.entry.restrictions.restriction'
			].concat(accountRestrictionSchemas));
		});

		it('adds account restrictions schemas', () => {
			// - TransactionType.ACCOUNT_ADDRESS_RESTRICTION
			const accountAddressRestrictionSchema = modelSchema['TransactionType.ACCOUNT_ADDRESS_RESTRICTION'];
			expect(Object.keys(accountAddressRestrictionSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(accountAddressRestrictionSchema).to.contain.all.keys([
				'version', 'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - TransactionType.ACCOUNT_MOSAIC_RESTRICTION
			const accountMosaicRestrictionSchema = modelSchema['TransactionType.ACCOUNT_MOSAIC_RESTRICTION'];
			expect(Object.keys(accountMosaicRestrictionSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(accountMosaicRestrictionSchema).to.contain.all.keys([
				'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - TransactionType.ACCOUNT_OPERATION_RESTRICTION
			const accountOperationRestrictionSchema = modelSchema['TransactionType.ACCOUNT_OPERATION_RESTRICTION'];
			expect(Object.keys(accountOperationRestrictionSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(accountOperationRestrictionSchema).to.contain.all.keys([
				'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - accountRestrictions
			expect(Object.keys(modelSchema.accountRestrictions).length).to.equal(1);
			expect(modelSchema.accountRestrictions).to.contain.all.keys(['accountRestrictions']);

			// - accountRestriction.restrictions
			expect(Object.keys(modelSchema['accountRestriction.restrictions']).length).to.equal(3);
			expect(modelSchema['accountRestriction.restrictions']).to.contain.all.keys(['version', 'address', 'restrictions']);

			// - accountRestriction address, mosaic, and operation restrictions
			accountRestrictionSchemas.forEach(schema => {
				expect(Object.keys(modelSchema[schema]).length).to.equal(2);
				expect(modelSchema[schema]).to.contain.all.keys(['restrictionFlags', 'values']);
			});
		});

		it('uses the correct conditional account schema depending on restriction type', () => {
			// Arrange:
			const accountRestrictionSchema = modelSchema['accountRestriction.restrictions'].restrictions.schemaName;

			// Assert:
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.addressAllow }))
				.to.equal('accountRestriction.addressAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.addressBlock }))
				.to.equal('accountRestriction.addressAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.mosaicAllow }))
				.to.equal('accountRestriction.mosaicAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.mosaicBlock }))
				.to.equal('accountRestriction.mosaicAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.operationAllow }))
				.to.equal('accountRestriction.operationAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.operationBlock }))
				.to.equal('accountRestriction.operationAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: 99 }))
				.to.equal('accountRestriction.fallback');
		});

		it('adds mosaic restrictions system schemas', () => {
			// - TransactionType.MOSAIC_ADDRESS_RESTRICTION
			const mosaicAddressRestrictionSchema = modelSchema['TransactionType.MOSAIC_ADDRESS_RESTRICTION'];
			expect(Object.keys(mosaicAddressRestrictionSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(mosaicAddressRestrictionSchema).to.contain.all.keys([
				'mosaicId',
				'restrictionKey',
				'targetAddress',
				'previousRestrictionValue',
				'newRestrictionValue'
			]);

			// - TransactionType.MOSAIC_GLOBAL_RESTRICTION
			const mosaicGlobalRestrictionSchema = modelSchema['TransactionType.MOSAIC_GLOBAL_RESTRICTION'];
			expect(Object.keys(mosaicGlobalRestrictionSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 7);
			expect(mosaicGlobalRestrictionSchema).to.contain.all.keys([
				'mosaicId',
				'referenceMosaicId',
				'restrictionKey',
				'previousRestrictionValue',
				'newRestrictionValue',
				'previousRestrictionType',
				'newRestrictionType'
			]);

			// - mosaic restrictions
			expect(Object.keys(modelSchema.mosaicRestrictions).length).to.equal(2);
			expect(modelSchema.mosaicRestrictions).to.contain.all.keys(['id', 'mosaicRestrictionEntry']);

			// - mosaicRestriction.entry
			expect(Object.keys(modelSchema['mosaicRestrictions.entry']).length).to.equal(6);
			expect(modelSchema['mosaicRestrictions.entry']).to.contain.all.keys([
				'version', 'compositeHash', 'entryType', 'mosaicId', 'targetAddress', 'restrictions'
			]);

			// - mosaicRestrictions.entry.restrictions
			expect(Object.keys(modelSchema['mosaicRestrictions.entry.restrictions']).length).to.equal(3);
			expect(modelSchema['mosaicRestrictions.entry.restrictions']).to.contain.all.keys([
				'key', 'value', 'restriction'
			]);

			// - mosaicRestrictions.entry.restrictions.restriction
			expect(Object.keys(modelSchema['mosaicRestrictions.entry.restrictions.restriction']).length).to.equal(3);
			expect(modelSchema['mosaicRestrictions.entry.restrictions.restriction']).to.contain.all.keys([
				'referenceMosaicId', 'restrictionValue', 'restrictionType'
			]);
		});
	});
});
