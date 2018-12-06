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

const test = require('./utils/accountPropertiesDbTestUtils');
const { expect } = require('chai');

const removeMongoId = entity => {
	delete entity.meta.id;
	return entity;
};

describe('account properties db', () => {
	describe('account properties by address', () => {
		it('returns undefined for unknown account', () => {
			// Arrange:
			const { address } = test.random.account();
			const accountProperties1 = test.db.createAccountProperties(address, { numAddresses: 3, numMosaics: 3, numEntityTypes: 3 });

			// Assert:
			return test.db.runDbTest(
				accountProperties1,
				db => db.accountPropertiesByAddresses([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns found empty account properties for single account', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const randomAddress2 = test.random.account().address;
			const accountProperties1 = test.db.createAccountProperties(
				randomAddress1,
				{ numAddresses: 0, numMosaics: 0, numEntityTypes: 0 }
			);
			const accountProperties2 = test.db.createAccountProperties(
				address,
				{ numAddresses: 0, numMosaics: 0, numEntityTypes: 0 }
			);
			const accountProperties3 = test.db.createAccountProperties(
				randomAddress2,
				{ numAddresses: 0, numMosaics: 0, numEntityTypes: 0 }
			);

			// Assert:
			return test.db.runDbTest(
				[accountProperties1, accountProperties2, accountProperties3],
				db => db.accountPropertiesByAddresses([address]),
				entities => { expect(entities).to.deep.equal([removeMongoId(accountProperties2)]); }
			);
		});

		it('returns found populated account properties for single account', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const randomAddress2 = test.random.account().address;
			const accountProperties1 = test.db.createAccountProperties(
				randomAddress1,
				{ numAddresses: 3, numMosaics: 6, numEntityTypes: 2 }
			);
			const accountProperties2 = test.db.createAccountProperties(
				address,
				{ numAddresses: 3, numMosaics: 6, numEntityTypes: 2 }
			);
			const accountProperties3 = test.db.createAccountProperties(
				randomAddress2,
				{ numAddresses: 3, numMosaics: 6, numEntityTypes: 2 }
			);

			// Assert:
			return test.db.runDbTest(
				[accountProperties1, accountProperties2, accountProperties3],
				db => db.accountPropertiesByAddresses([address]),
				entities => { expect(entities[0]).to.deep.equal(removeMongoId(accountProperties2)); }
			);
		});

		it('returns found populated account properties for multiple accounts', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const accountProperties1 = test.db.createAccountProperties(
				address,
				{ numAddresses: 3, numMosaics: 6, numEntityTypes: 2 }
			);
			const accountProperties2 = test.db.createAccountProperties(
				randomAddress1,
				{ numAddresses: 3, numMosaics: 6, numEntityTypes: 2 }
			);
			const accountProperties3 = test.db.createAccountProperties(
				address,
				{ numAddresses: 1, numMosaics: 4, numEntityTypes: 3 }
			);

			// Assert:
			return test.db.runDbTest(
				[accountProperties1, accountProperties2, accountProperties3],
				db => db.accountPropertiesByAddresses([address]),
				entities => { expect(entities).to.deep.equal([removeMongoId(accountProperties1), removeMongoId(accountProperties3)]); }
			);
		});
	});
});
