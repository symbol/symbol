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

const catapult = require('../../catapult-sdk/index');
const { convertToLong, buildOffsetCondition, longToUint64 } = require('../../db/dbUtils');

const { uint64 } = catapult.utils;

const createLatestConditions = (catapultDb, height) => {
	if (height) {
		return ({
			$and: [{ 'meta.latest': true }, {
				$or: [
					{ 'namespace.endHeight': convertToLong(-1) },
					{ 'namespace.endHeight': { $gt: height } }]
			}]
		});
	}
	return { 'meta.latest': true };
};

const addActiveFlag = (namespace, height) => {
	if (!namespace)
		return namespace;

	// What about calculated fields in mongo?
	const endHeightUint64 = longToUint64(namespace.namespace.endHeight);
	const heightUint64 = longToUint64(convertToLong(height));
	namespace.meta.active = 1 === uint64.compare(endHeightUint64, heightUint64);
	return namespace;
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
	 * @param {number} aliasType Namespace alias type
	 * @param {module:utils/uint64~uint64} level0 Namespace level0
	 * @param {Uint8Array} ownerAddress Namespace owner address
	 * @param {number} registrationType Namespace registration type
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise<object>} Namespaces page.
	 */
	async namespaces(aliasType, level0, ownerAddress, registrationType, options) {
		const sortingOptions = { id: '_id' };
		let conditions = {};
		const { height } = await this.catapultDb.chainStatisticCurrent();
		const activeConditions = createLatestConditions(this.catapultDb);
		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== aliasType)
			conditions['namespace.alias.type'] = aliasType;

		if (undefined !== level0)
			conditions['namespace.level0'] = convertToLong(level0);

		if (undefined !== ownerAddress)
			conditions['namespace.ownerAddress'] = Buffer.from(ownerAddress);

		if (undefined !== registrationType)
			conditions['namespace.registrationType'] = registrationType;

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };
		return this.catapultDb.queryPagedDocuments(
			{ $and: [activeConditions, conditions] },
			[],
			sortConditions,
			'namespaces',
			options,
			n => addActiveFlag(n, height)
		);
	}

	/**
	 * Retrieves a namespace.
	 * @param {module:utils/uint64~uint64} id Namespace id.
	 * @returns {Promise<object>} Namespace.
	 */
	async namespaceById(id) {
		const { height } = await this.catapultDb.chainStatisticCurrent();
		const activeConditions = createLatestConditions(this.catapultDb);
		const topLevelConditions = { $or: [] };

		for (let level = 0; 3 > level; ++level) {
			const conditions = [];
			conditions.push(activeConditions);
			conditions.push({ [`namespace.level${level}`]: convertToLong(id) });
			conditions.push({ 'namespace.depth': level + 1 });
			topLevelConditions.$or.push({ $and: conditions });
		}

		return this.catapultDb.queryDocument('namespaces', topLevelConditions)
			.then(this.catapultDb.sanitizer.renameId).then(n => addActiveFlag(n, height));
	}

	/**
	 * Retrieves non expired namespaces aliasing mosaics or addresses.
	 * @param {Array<module:catapult.model.namespace.aliasType>} aliasType Alias type.
	 * @param {*} ids Set of mosaic or address ids.
	 * @returns {Promise<Array<object>>} Active namespaces aliasing ids.
	 */
	async activeNamespacesWithAlias(aliasType, ids) {
		const aliasFilterCondition = {
			[catapult.model.namespace.aliasType.mosaic]: () => ({ 'namespace.alias.mosaicId': { $in: ids.map(convertToLong) } }),
			[catapult.model.namespace.aliasType.address]: () => ({ 'namespace.alias.address': { $in: ids.map(id => Buffer.from(id)) } })
		};
		const { height } = await this.catapultDb.chainStatisticCurrent();
		const activeConditions = await createLatestConditions(this.catapultDb, height);

		const conditions = { $and: [] };
		conditions.$and.push(aliasFilterCondition[aliasType]());
		conditions.$and.push({ 'namespace.alias.type': aliasType });
		conditions.$and.push(activeConditions);

		return this.catapultDb.queryDocuments('namespaces', conditions).then(ns => ns.map(n => addActiveFlag(n, height)));
	}

	// endregion

	/**
	 * Retrieves transactions that registered the specified namespaces.
	 * @param {Array<module:utils/uint64~uint64>} namespaceIds Namespace ids.
	 * @returns {Promise<Array<object>>} Register namespace transactions.
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
