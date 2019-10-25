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

const AccountType = require('../AccountType');
const MongoDb = require('mongodb');

const { Long } = MongoDb;

class MosaicDb {
	/**
	 * Creates MosaicDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region mosaic retrieval

	/**
	 * Retrieves mosaics.
	 * @param {Array.<module:catapult.utils/uint64~uint64>} ids Mosaic ids.
	 * @returns {Promise.<array>} Mosaics.
	 */
	mosaicsByIds(ids) {
		const mosaicIds = ids.map(id => new Long(id[0], id[1]));
		const conditions = { 'mosaic.id': { $in: mosaicIds } };
		const collection = this.catapultDb.database.collection('mosaics');
		return collection.find(conditions)
			.sort({ _id: -1 })
			.toArray()
			.then(entities => Promise.resolve(this.catapultDb.sanitizer.deleteIds(entities)));
	}

	/**
	 * Retrieves mosaics owned by specified owners.
	 * @param {module:db/AccountType} type Type of account ids.
	 * @param {array<object>} accountIds Account ids.
	 * @returns {Promise.<array>} Owned mosaics.
	 */
	mosaicsByOwners(type, accountIds) {
		const buffers = accountIds.map(accountId => Buffer.from(accountId));
		const fieldName = (AccountType.publicKey === type) ? 'mosaic.ownerPublicKey' : 'mosaic.ownerAddress';
		const conditions = { [fieldName]: { $in: buffers } };

		return this.catapultDb.queryDocuments('mosaics', conditions)
			.then(mosaics => mosaics.map(mosaic => mosaic.mosaic));
	}

	// endregion
}

module.exports = MosaicDb;
