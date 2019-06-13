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

const catapult = require('catapult-sdk');
const dbUtils = require('../../db/dbUtils');
const MongoDb = require('mongodb');

const { convertToLong } = dbUtils;
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
		const conditions = { 'mosaic.mosaicId': { $in: mosaicIds } };
		const collection = this.catapultDb.database.collection('mosaics');
		return collection.find(conditions)
			.sort({ _id: -1 })
			.toArray()
			.then(entities => Promise.resolve(this.catapultDb.sanitizer.copyAndDeleteIds(entities)));
	}

	// endregion

	/**
	 * Retrieves non expired namespaces aliasing specified mosaics.
	 * @param {Array.<module:catapult.utils/uint64~uint64>} mosaicsIds Mosaic ids.
	 * @returns {Promise.<array>} Mosaic alias namespaces.
	 */
	activeNamespacesByMosaicsIds(mosaicsIds) {
		const namespaceAliasType = catapult.model.namespace.aliasType.mosaic;
		const blockCountPromise = this.catapultDb.database.collection('blocks').count();

		return blockCountPromise.then(numBlocks => {
			const conditions = { $and: [] };
			conditions.$and.push({ 'namespace.alias.mosaicId': { $in: mosaicsIds.map(convertToLong) } });
			conditions.$and.push({ 'namespace.alias.type': namespaceAliasType });
			conditions.$and.push({
				$or: [
					{ 'namespace.endHeight': convertToLong(-1) },
					{ 'namespace.endHeight': { $gt: numBlocks } }]
			});

			return this.catapultDb.queryDocuments('namespaces', conditions);
		});
	}

	/**
	 * Retrieves transactions that registered the specified namespaces.
	 * @param {Array.<module:catapult.utils/uint64~uint64>} namespaceIds Namespace ids.
	 * @returns {Promise.<array>} Register namespace transactions.
	 */
	registerNamespaceTransactionsByNamespaceIds(namespaceIds) {
		const type = catapult.model.EntityType.registerNamespace;
		const conditions = { $and: [] };
		conditions.$and.push({ 'transaction.namespaceId': { $in: namespaceIds } });
		conditions.$and.push({ 'transaction.type': type });
		return this.catapultDb.queryDocuments('transactions', conditions);
	}
}

module.exports = MosaicDb;
