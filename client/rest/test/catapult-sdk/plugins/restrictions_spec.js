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
				'accountRestrictionAddress',
				'accountRestrictionMosaic',
				'accountRestrictionOperation',
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
			// - accountRestrictionAddress
			expect(Object.keys(modelSchema.accountRestrictionAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountRestrictionAddress).to.contain.all.keys([
				'version', 'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - accountRestrictionMosaic
			expect(Object.keys(modelSchema.accountRestrictionMosaic).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountRestrictionMosaic).to.contain.all.keys([
				'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - accountRestrictionOperation
			expect(Object.keys(modelSchema.accountRestrictionOperation).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountRestrictionOperation).to.contain.all.keys([
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
			// - mosaic restriction address transaction
			expect(Object.keys(modelSchema.mosaicRestrictionAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.mosaicRestrictionAddress).to.contain.all.keys([
				'mosaicId',
				'restrictionKey',
				'targetAddress',
				'previousRestrictionValue',
				'newRestrictionValue'
			]);

			// - mosaic restriction global transaction
			expect(Object.keys(modelSchema.mosaicRestrictionGlobal).length).to.equal(Object.keys(modelSchema.transaction).length + 7);
			expect(modelSchema.mosaicRestrictionGlobal).to.contain.all.keys([
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
