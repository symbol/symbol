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

const { metal } = require('./metal');
const { convertToLong, buildOffsetCondition, longToUint64 } = require('../../db/dbUtils');

class MetadataDb {
	/**
	 * Creates MetadataDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	/**
	 * Retrieves filtered and paginated metadata.
	 * @param {Uint8Array} sourceAddress Metadata source address
	 * @param {Uint8Array} targetAddress Metadata target address
	 * @param {module:utils/uint64~uint64} scopedMetadataKey Metadata scoped key
	 * @param {module:utils/uint64~uint64} targetId Metadata target id
	 * @param {number} metadataType Metadata type
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise<object>} Metadata page.
	 */
	metadata(sourceAddress, targetAddress, scopedMetadataKey, targetId, metadataType, options) {
		const sortingOptions = { id: '_id' };

		let conditions = {};

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== sourceAddress)
			conditions['metadataEntry.sourceAddress'] = Buffer.from(sourceAddress);

		if (undefined !== targetAddress)
			conditions['metadataEntry.targetAddress'] = Buffer.from(targetAddress);

		if (undefined !== scopedMetadataKey)
			conditions['metadataEntry.scopedMetadataKey'] = convertToLong(scopedMetadataKey);

		if (undefined !== targetId)
			conditions['metadataEntry.targetId'] = convertToLong(targetId);

		if (undefined !== metadataType)
			conditions['metadataEntry.metadataType'] = metadataType;

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };
		return this.catapultDb.queryPagedDocuments(conditions, [], sortConditions, 'metadata', options);
	}

	metadatasByCompositeHash(ids) {
		const compositeHashes = ids.map(id => Buffer.from(id));
		const conditions = { 'metadataEntry.compositeHash': { $in: compositeHashes } };
		const collection = this.catapultDb.database.collection('metadata');
		return collection.find(conditions)
			.sort({ _id: -1 })
			.toArray()
			.then(entities => Promise.resolve(this.catapultDb.sanitizer.renameIds(entities)));
	}

	async binDataByMetalId(metalId) {
		const metadatasByCompositeHash = await this.metadatasByCompositeHash([metal.extractCompositeHashFromMetalId(metalId)]);
		if (0 === metadatasByCompositeHash.length)
			throw Error(`could not get first chunk, it may mistake the metal ID: ${metalId}`);
		const { metadataEntry } = metadatasByCompositeHash[0];
		const chunks = [];
		let counter = 1;
		let hasMoreData = true;

		while (hasMoreData) {
			const options = {
				sortField: 'id', sortDirection: 1, pageSize: 100, pageNumber: counter
			};
			// eslint-disable-next-line no-await-in-loop
			const metadatas = await this.metadata(
				metadataEntry.sourceAddress.buffer,
				metadataEntry.targetAddress.buffer,
				undefined,
				metadataEntry.targetId,
				metadataEntry.metadataType,
				options
			);
			metadatas.data.forEach(e => {
				chunks.push({
					key: longToUint64(e.metadataEntry.scopedMetadataKey),
					value: e.metadataEntry.value.buffer
				});
			});

			hasMoreData = 0 < metadatas.data.length;
			counter++;
		}
		return metal.decode(longToUint64(metadataEntry.scopedMetadataKey), chunks);
	}
}

module.exports = MetadataDb;
