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

const MongoDb = require('mongodb');
const test = require('./receiptsDbTestUtils');
const { expect } = require('chai');

const { Long } = MongoDb;

describe('receipts db', () => {
	describe('address resolution statements by height', () => {
		// Arrange:
		const knownHeight = 4668;
		const addressResolutionsStatement1 = test.addressResolutionStatementDb.createAddressResolutionStatement(knownHeight);
		const addressResolutionsStatement2 = test.addressResolutionStatementDb.createAddressResolutionStatement(knownHeight);
		const addressResolutionsStatement3 = test.addressResolutionStatementDb.createAddressResolutionStatement(knownHeight + 1);

		it('returns empty array for non-existing address resolution statements in block', () =>
			// Assert:
			test.addressResolutionStatementDb.runDbTest(
				[addressResolutionsStatement1, addressResolutionsStatement2, addressResolutionsStatement3],
				db => db.statementsAtHeight(Long.fromNumber(knownHeight + 10), 'addressResolutionStatements'),
				entities => { expect(entities).to.deep.equal([]); }
			));

		it('returns address resolution statements in block at height', () =>
			// Assert:
			test.addressResolutionStatementDb.runDbTest(
				[addressResolutionsStatement1, addressResolutionsStatement2, addressResolutionsStatement3],
				db => db.statementsAtHeight(Long.fromNumber(knownHeight), 'addressResolutionStatements'),
				entities => { expect(entities).to.deep.equal([addressResolutionsStatement1, addressResolutionsStatement2]); }
			));
	});

	describe('mosaic resolution statements by height', () => {
		// Arrange:
		const knownHeight = 4668;
		const mosaicResolutionsStatement1 = test.mosaicResolutionStatementDb.createMosaicResolutionStatement(knownHeight);
		const mosaicResolutionsStatement2 = test.mosaicResolutionStatementDb.createMosaicResolutionStatement(knownHeight);
		const mosaicResolutionsStatement3 = test.mosaicResolutionStatementDb.createMosaicResolutionStatement(knownHeight + 1);

		it('returns empty array for non-existing mosaic resolution statements in block', () =>
			// Assert:
			test.mosaicResolutionStatementDb.runDbTest(
				[mosaicResolutionsStatement1, mosaicResolutionsStatement2, mosaicResolutionsStatement3],
				db => db.statementsAtHeight(Long.fromNumber(knownHeight + 10), 'mosaicResolutionStatements'),
				entities => { expect(entities).to.deep.equal([]); }
			));

		it('returns mosaic resolution statements in block at height', () =>
			// Assert:
			test.mosaicResolutionStatementDb.runDbTest(
				[mosaicResolutionsStatement1, mosaicResolutionsStatement2, mosaicResolutionsStatement3],
				db => db.statementsAtHeight(Long.fromNumber(knownHeight), 'mosaicResolutionStatements'),
				entities => { expect(entities).to.deep.equal([mosaicResolutionsStatement1, mosaicResolutionsStatement2]); }
			));
	});

	describe('transaction statements by height', () => {
		// Arrange:
		const knownHeight = 4668;
		const transactionStatement1 = test.transactionStatementDb.createTransactionStatement(knownHeight);
		const transactionStatement2 = test.transactionStatementDb.createTransactionStatement(knownHeight);
		const transactionStatement3 = test.transactionStatementDb.createTransactionStatement(knownHeight + 1);

		it('returns empty array for non-existing transaction statements in block', () =>
			// Assert:
			test.transactionStatementDb.runDbTest(
				[transactionStatement1, transactionStatement2, transactionStatement3],
				db => db.statementsAtHeight(Long.fromNumber(knownHeight + 10), 'transactionStatements'),
				entities => { expect(entities).to.deep.equal([]); }
			));

		it('returns transaction statements in block at height', () =>
			// Assert:
			test.transactionStatementDb.runDbTest(
				[transactionStatement1, transactionStatement2, transactionStatement3],
				db => db.statementsAtHeight(Long.fromNumber(knownHeight), 'transactionStatements'),
				entities => { expect(entities).to.deep.equal([transactionStatement1, transactionStatement2]); }
			));
	});
});
