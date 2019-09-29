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

const test = require('../../db/utils/dbTestUtils');
const MosaicRestrictionsDb = require('../../../src/plugins/mosaicRestrictions/MosaicRestrictionsDb');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { mosaicRestriction } = catapult.model;
const { Binary, ObjectId, Long } = MongoDb;

const createObjectId = id => new ObjectId(`${'00'.repeat(12)}${id}`.slice(-24));

const mosaicRestrictionsDbTestUtils = {

	sanitizeId: entity => { delete entity._id; return entity; },

	createGlobalMosaicRestriction: mosaicId => ({
		_id: createObjectId(Math.floor(Math.random() * 100000)),
		mosaicRestrictionEntry: {
			compositeHash: '',
			entryType: mosaicRestriction.restrictionType.global,
			mosaicId: new Long(mosaicId[0], mosaicId[1]),
			restrictions: [{ key: '', restriction: { referenceMosaicId: '', restrictionValue: '', restrictionType: 0 } }]
		}
	}),

	createAddressMosaicRestriction: (mosaicId, targetAddress) => ({
		_id: createObjectId(Math.floor(Math.random() * 100000)),
		mosaicRestrictionEntry: {
			compositeHash: '',
			entryType: mosaicRestriction.restrictionType.address,
			mosaicId: new Long(mosaicId[0], mosaicId[1]),
			targetAddress: new Binary(Buffer.from(targetAddress)),
			restrictions: [{ key: '', value: '' }]
		}
	}),

	runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) => test.db.runDbTest(
		dbEntities,
		'mosaicRestrictions',
		db => new MosaicRestrictionsDb(db),
		issueDbCommand,
		assertDbCommandResult
	)
};

module.exports = mosaicRestrictionsDbTestUtils;
