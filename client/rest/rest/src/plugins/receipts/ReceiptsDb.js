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

const { convertToLong, buildOffsetCondition, uniqueLongList } = require('../../db/dbUtils');
const catapult = require('catapult-sdk');

const { convert, uint64 } = catapult.utils;

const isNamespaceId = id => 0 !== (0x80 & convert.hexToUint8(uint64.toHex(id))[0]);

class ReceiptsDb {
	/**
	* Creates ReceiptsDb around CatapultDb.
	* @param {module:db/CatapultDb} db Catapult db instance.
	*/
	constructor(db) {
		this.catapultDb = db;
	}

	/**
	 * Retrieves filtered and paginated transaction statements.
	 * @param {object} filters Filters to be applied: `height`, `receiptType`, `recipientAddress`, `senderAddress`, `targetAddress`,
	 * `artifactId`.
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Transaction statements page.
	 */
	transactionStatements(filters, options) {
		const sortingOptions = { id: '_id' };

		let conditions = {};

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== filters.height)
			conditions['statement.height'] = convertToLong(filters.height);

		if (undefined !== filters.receiptType)
			conditions['statement.receipts.type'] = { $in: filters.receiptType };

		if (undefined !== filters.recipientAddress)
			conditions['statement.receipts.recipientAddress'] = Buffer.from(filters.recipientAddress);

		if (undefined !== filters.senderAddress)
			conditions['statement.receipts.senderAddress'] = Buffer.from(filters.senderAddress);

		if (undefined !== filters.targetAddress)
			conditions['statement.receipts.targetAddress'] = Buffer.from(filters.targetAddress);

		if (undefined !== filters.artifactId) {
			const artifactIdType = isNamespaceId(filters.artifactId) ? 'namespaceId' : 'mosaicId';
			conditions[[`statement.receipts.${artifactIdType}`]] = convertToLong(filters.artifactId);
		}

		if (undefined !== filters.fromHeight || undefined !== filters.toHeight) {
			const heightPath = 'statement.height';
			conditions[heightPath] = {};

			if (undefined !== filters.fromHeight)
				conditions[heightPath].$gte = convertToLong(filters.fromHeight);

			if (undefined !== filters.toHeight)
				conditions[heightPath].$lte = convertToLong(filters.toHeight);
		}

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };

		return this.catapultDb.queryPagedDocuments(conditions, [], sortConditions,
			'transactionStatements', options).then(page => this.addBlockMeta(page));
	}

	/**
	 * Retrieves filtered and paginated artifact resolution statements.
	 * @param {numeric} height Statement height.
	 * @param {Uint8Array} artifact Must be provided, determines the type of statements that are being fetched. May be `address`, or
	 * `mosaic`.
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Artifact statements page.
	 */
	artifactStatements(height, artifact, options) {
		const sortingOptions = { id: '_id' };

		let conditions = {};

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== height)
			conditions['statement.height'] = convertToLong(height);

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };
		return this.catapultDb.queryPagedDocuments(conditions, [], sortConditions,
			`${artifact}ResolutionStatements`, options).then(page => this.addBlockMeta(page));
	}

	/**
	 * It retrives and adds the blocks information to the statements' meta.
	 *
	 * The block information includes the its timestamp
	 *
	 * @param {object} page the page without meta in the items.
	 * @returns {Promise<{pagination, data}>} the page with the added block's meta to the items.
	 */
	async addBlockMeta(page) {
		const blockHeights = uniqueLongList(
			page.data.map(pageItem => pageItem.statement.height)
		);
		const blocks = await this.catapultDb.blocksAtHeights(blockHeights,
			{ 'block.timestamp': 1, 'block.height': 1 });
		const data = page.data.map(pageItem => {
			const statementBlock = blocks.find(
				blockInfo => blockInfo.block.height.equals(
					pageItem.statement.height
				)
			);
			if (!statementBlock) {
				throw new Error(
					`Cannot find block with height ${pageItem.statement.height.toString()}`
				);
			}
			if (!statementBlock.block.timestamp) {
				throw new Error(
					`Cannot find timestamp in block with height ${pageItem.statement.height.toString()}`
				);
			}
			// creates a copy of the page item with the added timestamp to the meta field.
			return { meta: { ...pageItem.meta, timestamp: statementBlock.block.timestamp }, ...pageItem };
		});
		return { data, pagination: page.pagination };
	}
}

module.exports = ReceiptsDb;
