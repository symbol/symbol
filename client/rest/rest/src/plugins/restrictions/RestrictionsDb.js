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

const { convertToLong, buildOffsetCondition } = require('../../db/dbUtils');

class RestrictionsDb {
	/**
	 * Creates RestrictionsDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	/**
	 * Retrieves account restrictions for the given addresses.
	 * @param {array<object>} addresses Given addresses.
	 * @returns {Promise.<array>} Owned account restrictions.
	 */
	accountRestrictionsByAddresses(addresses) {
		const buffers = addresses.map(address => Buffer.from(address));
		const conditions = { 'accountRestrictions.address': { $in: buffers } };
		return this.catapultDb.queryDocuments('accountRestrictions', conditions);
	}

	/**
	 * Retrieves filtered and paginated mosaic restrictions.
	 * @param {Uint64} mosaicId Mosaic id
	 * @param {uint} entryType Mosaic restriction type
	 * @param {Uint8Array} targetAddress Mosaic restriction target address
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Mosaic restrictions page.
	 */
	mosaicRestrictions(mosaicId, entryType, targetAddress, options) {
		const sortingOptions = { id: '_id' };

		let conditions = {};

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== mosaicId)
			conditions['mosaicRestrictionEntry.mosaicId'] = convertToLong(mosaicId);

		if (undefined !== entryType)
			conditions['mosaicRestrictionEntry.entryType'] = entryType;

		if (undefined !== targetAddress)
			conditions['mosaicRestrictionEntry.targetAddress'] = Buffer.from(targetAddress);

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };
		return this.catapultDb.queryPagedDocuments(conditions, [], sortConditions, 'mosaicRestrictions', options);
	}
}

module.exports = RestrictionsDb;
