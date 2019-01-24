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

const createLong = value => {
	if (Number.isInteger(value))
		return MongoDb.Long.fromNumber(value);

	// if value is an array, assume it is a uint64
	return Array.isArray(value) ? new MongoDb.Long(value[0], value[1]) : value;
};

class ReceiptsDb {
	/**
	* Creates ReceiptsDb around CatapultDb.
	* @param {module:db/CatapultDb} db Catapult db instance.
	*/
	constructor(db) {
		this.catapultDb = db;
	}

	/**
	* Retrieves all address resolution statements in a given block.
	* @param {module:catapult.utils/uint64~uint64} height The given block height.
	* @returns {Promise.<array>} Address resolution statements in a block.
	*/
	addressResolutionStatementsAtHeight(height) {
		return this.catapultDb.queryDocuments('addressResolutionStatements', { height: createLong(height) });
	}

	/**
	* Retrieves all mosaic resolution statements in a given block.
	* @param {module:catapult.utils/uint64~uint64} height The given block height.
	* @returns {Promise.<array>} Mosaic resolution statements in a block.
	*/
	mosaicResolutionStatementsAtHeight(height) {
		return this.catapultDb.queryDocuments('mosaicResolutionStatements', { height: createLong(height) });
	}

	/**
	* Retrieves all transaction statements in a given block.
	* @param {module:catapult.utils/uint64~uint64} height The given block height.
	* @returns {Promise.<array>} Transaction statements in a block.
	*/
	transactionStatementsAtHeight(height) {
		return this.catapultDb.queryDocuments('transactionStatements', { height: createLong(height) });
	}
}

module.exports = ReceiptsDb;
