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

/** @module db/CatapultDb */

const connector = require('./connector');
const { convertToLong } = require('./dbUtils');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { EntityType } = catapult.model;
const { ObjectId } = MongoDb;

const isAggregateType = document => EntityType.aggregateComplete === document.transaction.type
	|| EntityType.aggregateBonded === document.transaction.type;

const createSanitizer = () => ({
	copyAndDeleteId: dbObject => {
		if (dbObject) {
			Object.assign(dbObject.meta, { id: dbObject._id });
			delete dbObject._id;
		}

		return dbObject;
	},

	copyAndDeleteIds: dbObjects => {
		dbObjects.forEach(dbObject => {
			Object.assign(dbObject.meta, { id: dbObject._id });
			delete dbObject._id;
		});

		return dbObjects;
	},

	deleteId: dbObject => {
		if (dbObject)
			delete dbObject._id;

		return dbObject;
	},

	deleteIds: dbObjects => {
		dbObjects.forEach(dbObject => {
			delete dbObject._id;
		});
		return dbObjects;
	}
});

const mapToPromise = dbObject => Promise.resolve(null === dbObject ? undefined : dbObject);

const buildBlocksFromOptions = (height, numBlocks, chainHeight) => {
	const one = convertToLong(1);
	const startHeight = height.isZero() ? chainHeight.subtract(numBlocks).add(one) : height;

	// In all cases endHeight is actually max height + 1.
	const calculatedEndHeight = startHeight.add(numBlocks);
	const chainEndHeight = chainHeight.add(one);

	const endHeight = calculatedEndHeight.lessThan(chainEndHeight) ? calculatedEndHeight : chainEndHeight;
	return { startHeight, endHeight, numBlocks: endHeight.subtract(startHeight).toNumber() };
};

const getBoundedPageSize = (pageSize, pagingOptions) =>
	Math.max(pagingOptions.pageSizeMin, Math.min(pagingOptions.pageSizeMax, pageSize || pagingOptions.pageSizeDefault));

class CatapultDb {
	// region construction / connect / disconnect

	constructor(options) {
		this.networkId = options.networkId;
		if (!this.networkId)
			throw Error('network id is required');

		this.pagingOptions = {
			pageSizeMin: options.pageSizeMin,
			pageSizeMax: options.pageSizeMax,
			pageSizeDefault: options.pageSizeDefault
		};
		this.sanitizer = createSanitizer();
	}

	connect(url, dbName) {
		return connector.connectToDatabase(url, dbName)
			.then(client => {
				this.client = client;
				this.database = client.db();
			});
	}

	close() {
		if (!this.database)
			return Promise.resolve();

		return new Promise(resolve => {
			this.client.close(resolve);
			this.client = undefined;
			this.database = undefined;
		});
	}

	// endregion

	// region helpers

	queryDocument(collectionName, conditions, projection) {
		const collection = this.database.collection(collectionName);
		return collection.findOne(conditions, { projection })
			.then(mapToPromise);
	}

	queryDocuments(collectionName, conditions) {
		const collection = this.database.collection(collectionName);
		return collection.find(conditions)
			.toArray()
			.then(this.sanitizer.deleteIds);
	}

	queryRawDocuments(collectionName, conditions) {
		return this.database.collection(collectionName).find(conditions).toArray();
	}

	queryDocumentsAndCopyIds(collectionName, conditions, options = {}) {
		const collection = this.database.collection(collectionName);
		return collection.find(conditions)
			.project(options.projection)
			.toArray()
			.then(this.sanitizer.copyAndDeleteIds);
	}

	queryPagedDocuments(collectionName, conditions, id, pageSize, options = {}) {
		const sortOrder = options.sortOrder || -1;
		let allConditions = conditions;

		if (id) {
			allConditions = {
				$and: [
					conditions,
					{ _id: { [0 > sortOrder ? '$lt' : '$gt']: new ObjectId(id) } }
				]
			};
		}

		const collection = this.database.collection(collectionName);
		return collection.find(allConditions)
			.project(options.projection)
			.sort({ _id: sortOrder })
			.limit(getBoundedPageSize(pageSize, this.pagingOptions))
			.toArray();
	}

	// endregion

	// region retrieval

	/**
	 * Retrieves sizes of database collections.
	 * @returns {Promise} Promise that resolves to the sizes of collections in the database.
	 */
	storageInfo() {
		const blockCountPromise = this.database.collection('blocks').countDocuments();
		const transactionCountPromise = this.database.collection('transactions').countDocuments();
		const accountCountPromise = this.database.collection('accounts').countDocuments();
		return Promise.all([blockCountPromise, transactionCountPromise, accountCountPromise])
			.then(storageInfo => ({ numBlocks: storageInfo[0], numTransactions: storageInfo[1], numAccounts: storageInfo[2] }));
	}

	chainStatistic() {
		return this.queryDocument('chainStatistic', {}, { _id: 0 });
	}

	chainStatisticCurrent() {
		return this.queryDocument('chainStatistic', {}, { _id: 0 })
			.then(chainStatistic => chainStatistic.current);
	}

	/**
	 * Retrieves filtered and paginated blocks.
	 * @param {Uint8Array} signerPublicKey Filters by signer public key
	 * @param {Uint8Array} beneficiaryAddress Filters by beneficiary address
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`.
	 * @returns {Promise.<object>} Blocks page.
	 */
	blocks(signerPublicKey, beneficiaryAddress, options) {
		const conditions = [];

		// it is assumed that sortField will always be an `id` for now - this will need to be redesigned when it gets upgraded
		// in fact, offset logic should be moved to `queryPagedDocuments`
		if (options.offset)
			conditions.push({ [options.sortField]: { [1 === options.sortDirection ? '$gt' : '$lt']: new ObjectId(options.offset) } });

		if (signerPublicKey)
			conditions.push({ 'block.signerPublicKey': Buffer.from(signerPublicKey) });

		if (beneficiaryAddress)
			conditions.push({ 'block.beneficiaryAddress': Buffer.from(beneficiaryAddress) });

		const sortConditions = { $sort: { [options.sortField]: options.sortDirection } };

		return this.queryPagedDocuments_2(conditions, [], sortConditions, 'blocks', options);
	}

	blockAtHeight(height) {
		return this.queryDocument(
			'blocks',
			{ 'block.height': convertToLong(height) },
			{ 'meta.transactionMerkleTree': 0, 'meta.statementMerkleTree': 0 }
		).then(this.sanitizer.deleteId);
	}

	blockWithMerkleTreeAtHeight(height, merkleTreeName) {
		const blockMerkleTreeNames = ['transactionMerkleTree', 'statementMerkleTree'];
		const excludedMerkleTrees = {};
		blockMerkleTreeNames.filter(merkleTree => merkleTree !== merkleTreeName)
			.forEach(merkleTree => { excludedMerkleTrees[`meta.${merkleTree}`] = 0; });
		return this.queryDocument('blocks', { 'block.height': convertToLong(height) }, excludedMerkleTrees)
			.then(this.sanitizer.deleteId);
	}

	blocksFrom(height, numBlocks) {
		if (0 === numBlocks)
			return Promise.resolve([]);

		return this.chainStatisticCurrent().then(chainStatistic => {
			const blockCollection = this.database.collection('blocks');
			const options = buildBlocksFromOptions(convertToLong(height), convertToLong(numBlocks), chainStatistic.height);

			return blockCollection.find({ 'block.height': { $gte: options.startHeight, $lt: options.endHeight } })
				.project({ 'meta.transactionMerkleTree': 0, 'meta.statementMerkleTree': 0 })
				.sort({ 'block.height': -1 })
				.toArray()
				.then(this.sanitizer.deleteIds)
				.then(blocks => Promise.resolve(blocks));
		});
	}


	/**
	 * Retrieves the fee multiplier for the last (higher on the chain) numBlocks blocks
	 * @param {int} numBlocks Number of blocks to retrieve.
	 * @returns {Promise} Promise that resolves to feeMultiplier array
	 */
	latestBlocksFeeMultiplier(numBlocks) {
		if (0 === numBlocks)
			return Promise.resolve([]);

		return this.database.collection('blocks').find()
			.sort({ 'block.height': -1 })
			.limit(numBlocks)
			.project({ 'block.feeMultiplier': 1 })
			.toArray()
			.then(blocks => Promise.resolve(blocks.map(block => block.block.feeMultiplier)));
	}

	queryDependentDocuments(collectionName, aggregateIds) {
		if (0 === aggregateIds.length)
			return Promise.resolve([]);

		return this.queryDocumentsAndCopyIds(collectionName, { 'meta.aggregateId': { $in: aggregateIds } });
	}

	queryTransactions(conditions, id, pageSize, options) {
		// don't expose private meta.addresses field
		const optionsWithProjection = Object.assign({ projection: { 'meta.addresses': 0 } }, options);

		// filter out dependent documents
		const collectionName = (options || {}).collectionName || 'transactions';
		const transactionConditions = { $and: [{ 'meta.aggregateId': { $exists: false } }, conditions] };

		return this.queryPagedDocuments(collectionName, transactionConditions, id, pageSize, optionsWithProjection)
			.then(this.sanitizer.copyAndDeleteIds)
			.then(transactions => {
				const aggregateIds = [];
				const aggregateIdToTransactionMap = {};
				transactions
					.filter(isAggregateType)
					.forEach(document => {
						const aggregateId = document.meta.id;
						aggregateIds.push(aggregateId);
						aggregateIdToTransactionMap[aggregateId.toString()] = document.transaction;
					});

				return this.queryDependentDocuments(collectionName, aggregateIds).then(dependentDocuments => {
					dependentDocuments.forEach(dependentDocument => {
						const transaction = aggregateIdToTransactionMap[dependentDocument.meta.aggregateId];
						if (!transaction.transactions)
							transaction.transactions = [];

						transaction.transactions.push(dependentDocument);
					});

					return transactions;
				});
			});
	}

	/**
	 * Makes a paginated query with the provided arguments.
	 * @param {array<object>} queryConditions The conditions that determine the query results, may be empty.
	 * @param {array<string>} removedFields Field names to be hidden from the query results, may be empty.
	 * @param {object} sortConditions Condition that describes the order of the results, must be set.
	 * @param {string} collectionName Name of the collection to be queried.
	 * @param {object} options Pagination options, must contain `pageSize` and `pageNumber` (starting at 1).
	 * @returns {Promise.<object>} Page result, contains the attributes `data` with the actual results, and `paging` with pagination
	 * metadata - which is comprised of: `totalEntries`, `pageNumber`, and `pageSize`.
	 */
	queryPagedDocuments_2(queryConditions, removedFields, sortConditions, collectionName, options) {
		const conditions = [];
		if (queryConditions.length)
			conditions.push(1 === queryConditions.length ? { $match: queryConditions[0] } : { $match: { $and: queryConditions } });

		conditions.push(sortConditions);

		const { pageSize } = options;
		const pageIndex = options.pageNumber - 1;

		const facet = [
			{ $skip: pageSize * pageIndex },
			{ $limit: pageSize }
		];

		// rename _id to id
		facet.push({ $set: { id: '$_id' } });
		removedFields.push('_id');

		if (0 < Object.keys(removedFields).length)
			facet.push({ $unset: removedFields });

		conditions.push({
			$facet: {
				data: facet,
				pagination: [
					{ $count: 'totalEntries' },
					{
						$set: {
							pageNumber: options.pageNumber,
							pageSize
						}
					}
				]
			}
		});

		return this.database.collection(collectionName)
			.aggregate(conditions, { promoteLongs: false })
			.toArray()
			.then(result => {
				const formattedResult = result[0];

				// when query is empty, mongodb does not fill the pagination info
				if (!formattedResult.pagination.length)
					formattedResult.pagination = { totalEntries: 0, pageNumber: options.pageNumber, pageSize };
				else
					formattedResult.pagination = formattedResult.pagination[0];

				formattedResult.pagination.totalPages = Math.ceil(
					formattedResult.pagination.totalEntries / formattedResult.pagination.pageSize
				);

				return formattedResult;
			});
	}

	/**
	 * Retrieves filtered and paginated transactions.
	 * @param {object} filters Filters to be applied: `address` for an involved address in the query, `signerPublicKey`, `recipientAddress`,
	 * `state`, `height`, `embedded`, `transactionTypes` array of uint. If `address` is provided, other account related filters are omitted.
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize`.
	 * and `pageNumber`.
	 * @returns {Promise.<object>} Transactions page.
	 */
	transactions(filters, options) {
		const getCollectionName = (transactionStatus = 'confirmed') => {
			const collectionNames = {
				confirmed: 'transactions',
				unconfirmed: 'unconfirmedTransactions',
				partial: 'partialTransactions'
			};
			return collectionNames[transactionStatus];
		};
		const collectionName = getCollectionName(filters.state);

		const buildAccountConditions = () => {
			if (filters.address)
				return { 'meta.addresses': Buffer.from(filters.address) };

			const accountConditions = [];
			if (filters.signerPublicKey) {
				const signerPublicKeyCondition = { 'transaction.signerPublicKey': Buffer.from(filters.signerPublicKey) };
				accountConditions.push(signerPublicKeyCondition);
			}

			if (filters.recipientAddress) {
				const recipientAddressCondition = { 'transaction.recipientAddress': Buffer.from(filters.recipientAddress) };
				accountConditions.push(recipientAddressCondition);
			}

			if (Object.keys(accountConditions).length)
				return 1 < Object.keys(accountConditions).length ? { $and: accountConditions } : accountConditions[0];

			return undefined;
		};

		const buildConditions = () => {
			const conditions = [];

			// it is assumed that sortField will always be an `id` for now - this will need to be redesigned when it gets upgraded
			// in fact, offset logic should be moved to `queryPagedDocuments`
			if (options.offset)
				conditions.push({ [options.sortField]: { [1 === options.sortDirection ? '$gt' : '$lt']: new ObjectId(options.offset) } });

			if (filters.height)
				conditions.push({ 'meta.height': convertToLong(filters.height) });

			if (!filters.embedded)
				conditions.push({ 'meta.aggregateId': { $exists: false } });

			if (undefined !== filters.transactionTypes)
				conditions.push({ 'transaction.type': { $in: filters.transactionTypes } });

			const accountConditions = buildAccountConditions();
			if (accountConditions)
				conditions.push(accountConditions);

			return conditions;
		};

		const removedFields = ['meta.addresses'];
		const sortConditions = { $sort: { [options.sortField]: options.sortDirection } };
		const conditions = buildConditions();

		return this.queryPagedDocuments_2(conditions, removedFields, sortConditions, collectionName, options);
	}

	transactionsByIdsImpl(collectionName, conditions) {
		return this.queryDocumentsAndCopyIds(collectionName, conditions, { projection: { 'meta.addresses': 0 } })
			.then(documents => Promise.all(documents.map(document => {
				if (!document || !isAggregateType(document))
					return document;

				return this.queryDependentDocuments(collectionName, [document.meta.id]).then(dependentDocuments => {
					dependentDocuments.forEach(dependentDocument => {
						if (!document.transaction.transactions)
							document.transaction.transactions = [];

						document.transaction.transactions.push(dependentDocument);
					});

					return document;
				});
			})));
	}

	transactionsByIds(ids) {
		return this.transactionsByIdsImpl('transactions', { _id: { $in: ids.map(id => new ObjectId(id)) } });
	}

	transactionsByHashes(hashes) {
		return this.transactionsByIdsImpl('transactions', { 'meta.hash': { $in: hashes.map(hash => Buffer.from(hash)) } });
	}

	transactionsByHashesUnconfirmed(hashes) {
		return this.transactionsByIdsImpl('unconfirmedTransactions', { 'meta.hash': { $in: hashes.map(hash => Buffer.from(hash)) } });
	}

	transactionsByHashesPartial(hashes) {
		return this.transactionsByIdsImpl('partialTransactions', { 'meta.hash': { $in: hashes.map(hash => Buffer.from(hash)) } });
	}

	/**
	 * Return (id, name, parent) tuples for transactions with type and with id in set of ids.
	 * @param {*} ids Set of transaction ids.
	 * @param {*} transactionType Transaction type.
	 * @param {object} fieldNames Descriptor for fields used in query.
	 * @returns {Promise.<array>} Promise that is resolved when tuples are ready.
	 */
	findNamesByIds(ids, transactionType, fieldNames) {
		const queriedIds = ids.map(convertToLong);
		const conditions = {
			$match: {
				'transaction.type': transactionType,
				[`transaction.${fieldNames.id}`]: { $in: queriedIds }
			}
		};

		const grouping = {
			$group: {
				_id: `$transaction.${fieldNames.id}`,
				[fieldNames.id]: { $first: `$transaction.${fieldNames.id}` },
				[fieldNames.name]: { $first: `$transaction.${fieldNames.name}` },
				[fieldNames.parentId]: { $first: `$transaction.${fieldNames.parentId}` }
			}
		};

		const collection = this.database.collection('transactions');
		return collection.aggregate([conditions, grouping])
			.sort({ _id: -1 })
			.toArray()
			.then(this.sanitizer.deleteIds);
	}

	// region account retrieval

	accountsByIds(ids) {
		// id will either have address property or publicKey property set; in the case of publicKey, convert it to address
		const buffers = ids.map(id => Buffer.from((id.publicKey
			? catapult.model.address.publicKeyToAddress(id.publicKey, this.networkId) : id.address)));
		return this.queryDocuments('accounts', { 'account.address': { $in: buffers } })
			.then(entities => entities.map(accountWithMetadata => {
				const { account } = accountWithMetadata;
				if (0 < account.importances.length) {
					const importanceSnapshot = account.importances.shift();
					account.importance = importanceSnapshot.value;
					account.importanceHeight = importanceSnapshot.height;
				} else {
					account.importance = convertToLong(0);
					account.importanceHeight = convertToLong(0);
				}

				delete account.importances;
				return accountWithMetadata;
			}));
	}

	// endregion

	// region failed transaction

	/**
	 * Retrieves transaction results for the given hashes.
	 * @param {Array.<Uint8Array>} hashes Transaction hashes.
	 * @returns {Promise.<Array>} Promise that resolves to the array of hash / validation result pairs.
	 */
	transactionsByHashesFailed(hashes) {
		const buffers = hashes.map(hash => Buffer.from(hash));
		return this.queryDocuments('transactionStatuses', { 'status.hash': { $in: buffers } });
	}

	// endregion

	// region utils

	/**
	 * Retrieves account publickey projection for the given address.
	 * @param {Uint8Array} accountAddress Account address.
	 * @returns {Promise<Buffer>} Promise that resolves to the account public key.
	 */
	addressToPublicKey(accountAddress) {
		const conditions = { 'account.address': Buffer.from(accountAddress) };
		const projection = { 'account.publicKey': 1 };
		return this.queryDocument('accounts', conditions, projection);
	}

	// endregion
}

module.exports = CatapultDb;
