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

const ReceiptsDb = require('../../../src/plugins/receipts/ReceiptsDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const test = require('../../testUtils');
const MongoDb = require('mongodb');

const { Binary, Long } = MongoDb;

const createTransactionStatement = height => ({
	statement: {
		height: Long.fromNumber(height),
		source: {
			primaryId: 0,
			secondaryId: 0
		},
		receipts: [
			// balance change
			{
				version: 1,
				type: 8515,
				account: new Binary(test.random.publicKey()),
				mosaicId: Long.fromNumber(2345),
				amount: Long.fromNumber(6789)
			},
			// balance transfer
			{
				version: 1,
				type: 8516,
				senderAddress: new Binary(test.random.address()),
				recipientAddress: new Binary(test.random.address()),
				mosaicId: Long.fromNumber(9212),
				amount: Long.fromNumber(1314)
			},
			// artifact expiry
			{
				version: 1,
				type: 8517,
				artifactId: Long.fromNumber(1234)
			},
			// inflation
			{
				version: 1,
				type: 8518,
				mosaicId: Long.fromNumber(4532),
				amount: Long.fromNumber(200)
			}
		]
	}
});

const createAddressResolutionStatement = height => ({
	statement: {
		height: Long.fromNumber(height),
		unresolved: new Binary(test.random.address()),
		resolutionEntries: [
			{
				source: {
					primaryId: 23,
					secondaryId: 34
				},
				resolved: new Binary(test.random.address())
			}
		]
	}
});

const createMosaicResolutionStatement = height => ({
	statement: {
		height: Long.fromNumber(height),
		unresolved: Long.fromNumber(5432),
		resolutionEntries: [
			{
				source: {
					primaryId: 23,
					secondaryId: 34
				},
				resolved: Long.fromNumber(6789)
			}
		]
	}
});

const receiptsDbTestUtils = {
	transactionStatementDb: {
		createTransactionStatement,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(
				dbEntities,
				'transactionStatements',
				db => new ReceiptsDb(db),
				issueDbCommand,
				assertDbCommandResult
			)
	},
	addressResolutionStatementDb: {
		createAddressResolutionStatement,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(
				dbEntities,
				'addressResolutionStatements',
				db => new ReceiptsDb(db),
				issueDbCommand,
				assertDbCommandResult
			)
	},
	mosaicResolutionStatementDb: {
		createMosaicResolutionStatement,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(
				dbEntities,
				'mosaicResolutionStatements',
				db => new ReceiptsDb(db),
				issueDbCommand,
				assertDbCommandResult
			)
	}
};

Object.assign(receiptsDbTestUtils, test);

module.exports = receiptsDbTestUtils;
