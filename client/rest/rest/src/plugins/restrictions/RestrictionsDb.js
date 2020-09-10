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
const MongoDb = require('mongodb');

const { restriction } = catapult.model;
const { Long } = MongoDb;

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
	 * Retrieves mosaic restrictions of the given mosaic ids.
	 * @param {array<module:catapult.utils/uint64~uint64>} mosaicIds Given mosaic ids.
	 * @param {int} restrictionType Restriction type.
	 * @returns {Promise.<array>} Mosaic restrictions.
	 */
	mosaicRestrictionsByMosaicIds(mosaicIds, restrictionType) {
		const mosaicIdsLong = mosaicIds.map(mosaicId => new Long(mosaicId[0], mosaicId[1]));
		const conditions = {
			$and: [
				{ 'mosaicRestrictionEntry.mosaicId': { $in: mosaicIdsLong } },
				{ 'mosaicRestrictionEntry.entryType': restrictionType }
			]
		};

		return this.catapultDb.queryDocuments('mosaicRestrictions', conditions);
	}

	/**
	 * Retrieves mosaic address restrictions of the given mosaic id and target addresses.
	 * @param {array<module:catapult.utils/uint64~uint64>} mosaicId Given mosaic id.
	 * @param {array<module:catapult.model/address~address>} addresses Given addresses.
	 * @returns {Promise.<array>} Mosaic address restrictions.
	 */
	mosaicAddressRestrictions(mosaicId, addresses) {
		const addressesBuffers = addresses.map(address => Buffer.from(address));
		const conditions = {
			$and: [
				{ 'mosaicRestrictionEntry.mosaicId': new Long(mosaicId[0], mosaicId[1]) },
				{ 'mosaicRestrictionEntry.entryType': restriction.mosaicRestriction.restrictionType.address },
				{ 'mosaicRestrictionEntry.targetAddress': { $in: addressesBuffers } }
			]
		};

		return this.catapultDb.queryDocuments('mosaicRestrictions', conditions);
	}
}

module.exports = RestrictionsDb;
