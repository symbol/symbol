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

const test = require('./accountRestrictionsDbTestUtils');
const { expect } = require('chai');

const removeMongoId = entity => {
	delete entity.meta.id;
	return entity;
};

describe('account restrictions db', () => {
	describe('account restrictions by address', () => {
		it('returns undefined for unknown account', () => {
			// Arrange:
			const { address } = test.random.account();
			const accountRestrictions1 = test.db.createAccountRestrictions(address, { numAddresses: 3, numMosaics: 3, numOperations: 3 });

			// Assert:
			return test.db.runDbTest(
				accountRestrictions1,
				db => db.accountRestrictionsByAddresses([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns found empty account restrictions for single account', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const randomAddress2 = test.random.account().address;
			const accountRestrictions1 = test.db.createAccountRestrictions(
				randomAddress1,
				{ numAddresses: 0, numMosaics: 0, numOperations: 0 }
			);
			const accountRestrictions2 = test.db.createAccountRestrictions(
				address,
				{ numAddresses: 0, numMosaics: 0, numOperations: 0 }
			);
			const accountRestrictions3 = test.db.createAccountRestrictions(
				randomAddress2,
				{ numAddresses: 0, numMosaics: 0, numOperations: 0 }
			);

			// Assert:
			return test.db.runDbTest(
				[accountRestrictions1, accountRestrictions2, accountRestrictions3],
				db => db.accountRestrictionsByAddresses([address]),
				entities => { expect(entities).to.deep.equal([removeMongoId(accountRestrictions2)]); }
			);
		});

		it('returns found populated account restrictions for single account', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const randomAddress2 = test.random.account().address;
			const accountRestrictions1 = test.db.createAccountRestrictions(
				randomAddress1,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions2 = test.db.createAccountRestrictions(
				address,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions3 = test.db.createAccountRestrictions(
				randomAddress2,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);

			// Assert:
			return test.db.runDbTest(
				[accountRestrictions1, accountRestrictions2, accountRestrictions3],
				db => db.accountRestrictionsByAddresses([address]),
				entities => { expect(entities[0]).to.deep.equal(removeMongoId(accountRestrictions2)); }
			);
		});

		it('returns found populated account restrictions for multiple accounts', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const accountRestrictions1 = test.db.createAccountRestrictions(
				address,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions2 = test.db.createAccountRestrictions(
				randomAddress1,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions3 = test.db.createAccountRestrictions(
				address,
				{ numAddresses: 1, numMosaics: 4, numOperations: 3 }
			);

			// Assert:
			return test.db.runDbTest(
				[accountRestrictions1, accountRestrictions2, accountRestrictions3],
				db => db.accountRestrictionsByAddresses([address]),
				entities => { expect(entities).to.deep.equal([removeMongoId(accountRestrictions1), removeMongoId(accountRestrictions3)]); }
			);
		});
	});
});
