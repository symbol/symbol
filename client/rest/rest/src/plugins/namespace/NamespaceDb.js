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

const { convertToLong } = require('../../db/dbUtils');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { Long } = MongoDb;

const createActiveConditions = () => {
	const conditions = { $and: [{ 'meta.active': true }] };
	return conditions;
};

class NamespaceDb {
	/**
	 * Creates NamespaceDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region namespace retrieval

	/**
	 * Retrieves filtered and paginated namespaces.
	 * @param {Uint32} aliasType Namespace alias type
	 * @param {module:catapult.utils/uint64~uint64} level0 Namespace level0
	 * @param {Uint8Array} ownerAddress Namespace owner address
	 * @param {Uint32} registrationType Namespace registration type
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Namespaces page.
	 */
	namespaces(aliasType, level0, ownerAddress, registrationType, options) {
		const sortingOptions = { id: '_id' };
		const conditions = [];

		if (options.offset)
			conditions.push({ [sortingOptions[options.sortField]]: { [1 === options.sortDirection ? '$gt' : '$lt']: options.offset } });

		if (aliasType)
			conditions.push({ 'namespace.alias.type': aliasType });

		if (level0)
			conditions.push({ 'namespace.level0': new Long(level0[0], level0[1]) });

		if (ownerAddress)
			conditions.push({ 'namespace.ownerAddress': Buffer.from(ownerAddress) });

		if (registrationType)
			conditions.push({ 'namespace.registrationType': registrationType });

		// Returning only active namespaces
		conditions.push({ 'meta.active': true });

		const sortConditions = { $sort: { [sortingOptions[options.sortField]]: options.sortDirection } };
		return this.catapultDb.queryPagedDocuments_2(conditions, [], sortConditions, 'namespaces', options);
	}

	/**
	 * Retrieves a namespace.
	 * @param {module:catapult.utils/uint64~uint64} id Namespace id.
	 * @returns {Promise.<object>} Namespace.
	 */
	namespaceById(id) {
		const namespaceId = new Long(id[0], id[1]);
		const conditions = { $or: [] };

		for (let level = 0; 3 > level; ++level) {
			const conjunction = createActiveConditions();
			conjunction.$and.push({ [`namespace.level${level}`]: namespaceId });
			conjunction.$and.push({ 'namespace.depth': level + 1 });

			conditions.$or.push(conjunction);
		}

		return this.catapultDb.queryDocument('namespaces', conditions)
			.then(this.catapultDb.sanitizer.renameId);
	}

	/**
	 * Retrieves non expired namespaces aliasing mosaics or addresses.
	 * @param {Array.<module:catapult.model.namespace/aliasType>} aliasType Alias type.
	 * @param {*} ids Set of mosaic or address ids.
	 * @returns {Promise.<array>} Active namespaces aliasing ids.
	 */
	activeNamespacesWithAlias(aliasType, ids) {
		const aliasFilterCondition = {
			[catapult.model.namespace.aliasType.mosaic]: () => ({ 'namespace.alias.mosaicId': { $in: ids.map(convertToLong) } }),
			[catapult.model.namespace.aliasType.address]: () => ({ 'namespace.alias.address': { $in: ids.map(id => Buffer.from(id)) } })
		};

		return this.catapultDb.database.collection('blocks').countDocuments()
			.then(numBlocks => {
				const conditions = { $and: [] };
				conditions.$and.push(aliasFilterCondition[aliasType]());
				conditions.$and.push({ 'namespace.alias.type': aliasType });
				conditions.$and.push({
					$or: [
						{ 'namespace.endHeight': convertToLong(-1) },
						{ 'namespace.endHeight': { $gt: numBlocks } }]
				});

				return this.catapultDb.queryDocuments('namespaces', conditions);
			});
	}

	// endregion

	/**
	 * Retrieves transactions that registered the specified namespaces.
	 * @param {Array.<module:catapult.utils/uint64~uint64>} namespaceIds Namespace ids.
	 * @returns {Promise.<array>} Register namespace transactions.
	 */
	registerNamespaceTransactionsByNamespaceIds(namespaceIds) {
		const type = catapult.model.EntityType.registerNamespace;
		const conditions = { $and: [] };
		conditions.$and.push({ 'transaction.id': { $in: namespaceIds } });
		conditions.$and.push({ 'transaction.type': type });
		return this.catapultDb.queryDocuments('transactions', conditions);
	}
}

module.exports = NamespaceDb;
