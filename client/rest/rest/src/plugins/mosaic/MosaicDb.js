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

const { Long, ObjectId } = MongoDb;


class MosaicDb {
	/**
	 * Creates MosaicDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	/**
	 * Retrieves filtered and paginated mosaics.
	 * @param {Uint8Array} ownerAddress Mosaic owner address
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`.
	 * @returns {Promise.<object>} Mosaics page.
	 */
	mosaics(ownerAddress, options) {
		const conditions = [];

		// it is assumed that sortField will always be an `id` for now - this will need to be redesigned when it gets upgraded
		// in fact, offset logic should be moved to `queryPagedDocuments`
		if (options.offset)
			conditions.push({ [options.sortField]: { [1 === options.sortDirection ? '$gt' : '$lt']: new ObjectId(options.offset) } });

		if (ownerAddress)
			conditions.push({ 'mosaic.ownerAddress': Buffer.from(ownerAddress) });

		const sortConditions = { $sort: { [options.sortField]: options.sortDirection } };
		return this.catapultDb.queryPagedDocuments_2(conditions, [], sortConditions, 'mosaics', options);
	}

	/**
	 * Retrieves mosaics given their ids.
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
			.then(entities => Promise.resolve(this.catapultDb.sanitizer.renameIds(entities)));
	}
}

module.exports = MosaicDb;
