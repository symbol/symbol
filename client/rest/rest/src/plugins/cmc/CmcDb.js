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

const { convertToLong } = require('../../db/dbUtils');

class CmcDb {
	/**
	 * Creates NetworkCurrencyDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region NetworkCurrency retrieval
	/**
	 * Retrieves mosaics given their ids.
	 * @param {Array.<module:catapult.utils/uint64~uint64>} ids Mosaic ids.
	 * @returns {Promise.<array>} Mosaics.
	 */
	mosaicsByIds(ids) {
		const mosaicIds = ids.map(id => convertToLong(id));
		const conditions = { 'mosaic.id': { $in: mosaicIds } };
		const collection = this.catapultDb.database.collection('mosaics');
		return collection.find(conditions)
			.sort({ _id: -1 })
			.toArray()
			.then(entities => Promise.resolve(this.catapultDb.sanitizer.renameIds(entities)));
	}
	// endregion
}

module.exports = CmcDb;
