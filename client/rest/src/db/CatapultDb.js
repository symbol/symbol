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

/** @module db/CatapultDb */

const connector = require('./connector');
const { convertToLong, buildOffsetCondition, uniqueLongList } = require('./dbUtils');
const catapult = require('../catapult-sdk/index');
const MultisigDb = require('../plugins/multisig/MultisigDb');
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
	},

	renameId: dbObject => {
		if (dbObject) {
			dbObject.id = dbObject._id;
			delete dbObject._id;
		}

		return dbObject;
	},

	renameIds: dbObjects => {
		dbObjects.forEach(dbObject => {
			dbObject.id = dbObject._id;
			delete dbObject._id;
		});
		return dbObjects;
	}
});

const mapToPromise = dbObject => Promise.resolve(null === dbObject ? undefined : dbObject);

const pickTopImportance = wrappedAccount => {
	const { account } = wrappedAccount;
	if (0 < account.importances.length) {
		const importanceSnapshot = account.importances.shift();
		account.importance = importanceSnapshot.value;
		account.importanceHeight = importanceSnapshot.height;
	} else {
		account.importance = convertToLong(0);
		account.importanceHeight = convertToLong(0);
	}
	delete account.importances;
	return wrappedAccount;
};

const TransactionGroup = Object.freeze({
	confirmed: 'transactions',
	unconfirmed: 'unconfirmedTransactions',
	partial: 'partialTransactions'
});

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

	connect(url, dbName, connectionPoolSize, timeout) {
		return connector.connectToDatabase(url, dbName, connectionPoolSize || 10, timeout)
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

	queryDocumentsAndCopyIds(collectionName, conditions, options = {}) {
		const collection = this.database.collection(collectionName);
		return collection.find(conditions)
			.project(options.projection)
			.toArray()
			.then(this.sanitizer.renameIds);
	}

	// endregion

	// region retrieval

	/**
	 * Retrieves sizes of database collections.
	 * @returns {Promise} Promise that resolves to the sizes of collections in the database.
	 */
	storageInfo() {
		const blockCountPromise = this.database.collection('blocks').estimatedDocumentCount();
		const transactionCountPromise = this.database.collection('transactions').estimatedDocumentCount();
		const accountCountPromise = this.database.collection('accounts').estimatedDocumentCount();
		const dbStatsPromise = this.database.stats();

		const formatDbStats = dbStats => ({
			numIndexes: dbStats.indexes,
			numObjects: dbStats.objects,

			dataSize: dbStats.dataSize,
			indexSize: dbStats.indexSize,
			storageSize: dbStats.storageSize
		});

		return Promise.all([blockCountPromise, transactionCountPromise, accountCountPromise, dbStatsPromise])
			.then(storageInfo => ({
				numBlocks: storageInfo[0],
				numTransactions: storageInfo[1],
				numAccounts: storageInfo[2],
				database: formatDbStats(storageInfo[3])
			}));
	}

	chainStatisticCurrent() {
		return this.queryDocument('chainStatistic', {}, { _id: 0 })
			.then(chainStatistic => chainStatistic.current);
	}

	latestFinalizedBlock() {
		return this.database.collection('finalizedBlocks')
			.findOne({}, { sort: [['block.height', -1]], projection: { _id: 0 } })
			.then(mapToPromise);
	}

	/**
	 * Retrieves filtered and paginated blocks.
	 * @param {Uint8Array} signerPublicKey Filters by signer public key
	 * @param {Uint8Array} beneficiaryAddress Filters by beneficiary address
	 * @param {uint64} fromTimestamp Filters by fromTimestamp
	 * @param {uint64} toTimestamp Filters by toTimestamp
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Blocks page.
	 */
	blocks(signerPublicKey, beneficiaryAddress, fromTimestamp, toTimestamp, options) {
		const sortingOptions = {
			id: '_id',
			height: 'block.height'
		};

		let conditions = {};

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== signerPublicKey)
			conditions['block.signerPublicKey'] = Buffer.from(signerPublicKey);

		if (undefined !== beneficiaryAddress)
			conditions['block.beneficiaryAddress'] = Buffer.from(beneficiaryAddress);

		if (undefined !== fromTimestamp || undefined !== toTimestamp) {
			conditions['block.timestamp'] = {};
			if (undefined !== fromTimestamp)
				conditions['block.timestamp'].$gte = convertToLong(fromTimestamp);
			if (undefined !== toTimestamp)
				conditions['block.timestamp'].$lte = convertToLong(toTimestamp);
		}
		const removeFields = ['meta.transactionMerkleTree', 'meta.statementMerkleTree'];

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };

		return this.queryPagedDocuments(conditions, removeFields, sortConditions, 'blocks', options);
	}

	blockAtHeight(height) {
		return this.queryDocument(
			'blocks',
			{ 'block.height': convertToLong(height) },
			{ 'meta.transactionMerkleTree': 0, 'meta.statementMerkleTree': 0 }
		).then(this.sanitizer.renameId);
	}

	/**
	 * Returns the blocks (or a projection of the blocks) for the given heights.
	 * @param {Long[]} heights the array of long heights
	 * @param {object} projection optional projection, by default it excludes the transactionMerkleTree and statementMerkleTree fields
	 * @returns {Promise} with the blocks objects or projections.
	 */
	blocksAtHeights(heights, projection) {
		if (!heights.length)
			return Promise.resolve([]);

		return this.queryDocumentsAndCopyIds(
			'blocks',
			{ 'block.height': { $in: heights } },
			{ projection: projection || { 'meta.transactionMerkleTree': 0, 'meta.statementMerkleTree': 0 } }
		);
	}

	blockWithMerkleTreeAtHeight(height, merkleTreeName) {
		const blockMerkleTreeNames = ['transactionMerkleTree', 'statementMerkleTree'];
		const excludedMerkleTrees = {};
		blockMerkleTreeNames.filter(merkleTree => merkleTree !== merkleTreeName)
			.forEach(merkleTree => { excludedMerkleTrees[`meta.${merkleTree}`] = 0; });
		return this.queryDocument('blocks', { 'block.height': convertToLong(height) }, excludedMerkleTrees)
			.then(this.sanitizer.deleteId);
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

	/**
	 * Makes a paginated query with the provided arguments.
	 * @param {array<object>} queryConditions The conditions that determine the query results, may be empty.
	 * @param {array<string>} removedFields Field names to be hidden from the query results, may be empty.
	 * @param {object} sortConditions Condition that describes the order of the results, must be set.
	 * @param {string} collectionName Name of the collection to be queried.
	 * @param {object} options Pagination options, must contain `pageSize` and `pageNumber` (starting at 1).
	 * @param {function} mapper to transform each element of the page.
	 * @returns {Promise.<object>} Page result, contains the attributes `data` with the actual results, and `paging` with pagination
	 * metadata - which is comprised of: `pageNumber`, and `pageSize`.
	 */
	queryPagedDocuments(queryConditions, removedFields, sortConditions, collectionName, options, mapper) {
		const { pageSize } = options;
		const pageIndex = options.pageNumber - 1;

		const projection = {};
		removedFields.forEach(field => { projection[field] = 0; });

		return this.database.collection(collectionName)
			.find(queryConditions)
			.project(projection)
			.sort(sortConditions)
			.skip(pageSize * pageIndex)
			.limit(pageSize)
			.toArray()
			.then(this.sanitizer.renameIds)
			.then(xs => (mapper ? xs.map(mapper) : xs))
			.then(result => ({
				data: result,
				pagination: {
					pageNumber: options.pageNumber,
					pageSize
				}
			}));
	}

	/**
	 * Retrieves filtered and paginated transactions.
	 * @param {string} group Transactions group on which the query is made.
	 * @param {object} filters Filters to be applied: `address` for an involved address in the query, `signerPublicKey`, `recipientAddress`,
	 * `height`, `fromHeight`, `toHeight`, `embedded`, `transferMosaicId`, `fromTransferAmount`, `toTransferAmount`, `transactionTypes`
	 *  array of uint.
	 * If `address` is provided, other account related filters are omitted.
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Transactions page.
	 */
	async transactions(group, filters, options) {
		const sortingOptions = { id: '_id' };

		const buildAccountConditions = async () => {
			// Check multisig graph if address is used in search criteria for cosigning,
			// Then, show transactions for other cosigers.
			if (undefined !== filters.address) {
				if ('partial' === group) {
					const multisigEntries = await new MultisigDb(this).multisigsByAddresses([filters.address]);

					if (multisigEntries.length && multisigEntries[0].multisig.multisigAddresses.length)	{
						const buffers = multisigEntries[0].multisig.multisigAddresses.map(address => address.buffer);
						buffers.push(Buffer.from(filters.address));
						return { 'meta.addresses': { $in: buffers } };
					}
				}
				return { 'meta.addresses': Buffer.from(filters.address) };
			}
			const accountConditions = {};
			if (undefined !== filters.signerPublicKey)
				accountConditions['transaction.signerPublicKey'] = Buffer.from(filters.signerPublicKey);
			if (undefined !== filters.recipientAddress)
				accountConditions['transaction.recipientAddress'] = Buffer.from(filters.recipientAddress);
			return accountConditions;
		};

		const buildConditions = async () => {
			let conditions = {};

			const offsetCondition = buildOffsetCondition(options, sortingOptions);
			if (offsetCondition)
				conditions = Object.assign(conditions, offsetCondition);

			if (undefined !== filters.fromHeight || undefined !== filters.toHeight) {
				const heightPath = 'meta.height';
				conditions[heightPath] = {};

				if (undefined !== filters.fromHeight)
					conditions[heightPath].$gte = convertToLong(filters.fromHeight);

				if (undefined !== filters.toHeight)
					conditions[heightPath].$lte = convertToLong(filters.toHeight);
			}

			if (undefined !== filters.height)
				conditions['meta.height'] = convertToLong(filters.height);

			if (!filters.embedded)
				conditions['meta.aggregateId'] = { $exists: false };

			if (undefined !== filters.transactionTypes)
				conditions['transaction.type'] = { $in: filters.transactionTypes };

			/** transfer transaction specific filters */
			if (undefined !== filters.transferMosaicId)
				conditions['transaction.mosaics.id'] = convertToLong(filters.transferMosaicId);

			if (undefined !== filters.fromTransferAmount || undefined !== filters.toTransferAmount) {
				const amountPath = 'transaction.mosaics.amount';
				conditions[amountPath] = {};

				if (undefined !== filters.fromTransferAmount)
					conditions[amountPath].$gte = convertToLong(filters.fromTransferAmount);

				if (undefined !== filters.toTransferAmount)
					conditions[amountPath].$lte = convertToLong(filters.toTransferAmount);
			}

			const accountConditions = await buildAccountConditions();
			if (accountConditions)
				conditions = Object.assign(conditions, accountConditions);

			return conditions;
		};

		const removedFields = ['meta.addresses'];
		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };
		const conditions = await buildConditions();

		return this.queryPagedDocuments(conditions, removedFields, sortConditions, TransactionGroup[group], options).then(async page => {
			const data = await this.addBlockMetaToTransactionList(page.data);
			return ({
				...page,
				data
			});
		});
	}

	transactionsByIdsImpl(collectionName, conditions) {
		return this.queryDocumentsAndCopyIds(collectionName, conditions, { projection: { 'meta.addresses': 0 } })
			.then(list => this.addBlockMetaToTransactionList(list)).then(documents => Promise.all(documents.map(document => {
				if (!document || !isAggregateType(document))
					return document;

				return this.queryDependentDocuments(collectionName, [document.id]).then(dependentDocuments => {
					dependentDocuments.forEach(dependentDocument => {
						if (!document.transaction.transactions)
							document.transaction.transactions = [];
						if (document.meta.timestamp !== undefined && document.meta.feeMultiplier !== undefined) {
							dependentDocument.meta.timestamp = document.meta.timestamp;
							dependentDocument.meta.feeMultiplier = document.meta.feeMultiplier;
						}
						document.transaction.transactions.push(dependentDocument);
					});

					return document;
				});
			})));
	}

	/**
	 * It retrieves and adds the block information to the transactions' meta.
	 *
	 * The block information includes its timestamp and feeMultiplier
	 *
	 * @param {object[]} list the transaction list without the added block information.
	 * @returns {Promise<object[]>} the list with the added block information.
	 */
	addBlockMetaToTransactionList(list) {
		return this.addBlockMetaToEntityList(list, ['timestamp', 'feeMultiplier'], item => item.meta.height);
	}

	/**
	 * It retrieves and adds the block information to the entities' meta.
	 *
	 * @param {object[]} list the entity list without the added block information.
	 * @param {string[]} fields the list of fields to be be copied from the block's to the entity's meta.
	 * @param {Function} getHeight a function that returns the block's height of a given entity.
	 * @returns {Promise<object[]>} this list with the added block information.
	 */
	async addBlockMetaToEntityList(list, fields, getHeight) {
		const isValidHeight = height => height && 0 !== height.toInt();

		const blockHeights = uniqueLongList(list.map(item => getHeight(item)).filter(isValidHeight));

		const projection = {
			'block.height': 1,
			...fields.reduce((acc, field) => {
				acc[`block.${field}`] = 1;
				return acc;
			}, {})
		};
		const blocks = await this.blocksAtHeights(blockHeights, projection);

		return list.map(item => {
			const height = getHeight(item);
			if (!isValidHeight(height))
				return item;
			const block = blocks.find(blockInfo => blockInfo.block.height.equals(height));
			if (!block)
				throw new Error(`Cannot find block with height ${height.toString()}`);
			item.meta = item.meta || {};
			fields.forEach(field => {
				const value = block.block[field];
				if (value === undefined)
					throw new Error(`Cannot find ${field} in block with height ${height.toString()}`);
				item.meta[field] = value;
			});
			return item;
		});
	}

	transactionsByIds(group, ids) {
		return this.transactionsByIdsImpl(TransactionGroup[group], { _id: { $in: ids.map(id => new ObjectId(id)) } });
	}

	transactionsByHashes(group, hashes) {
		return this.transactionsByIdsImpl(TransactionGroup[group], { 'meta.hash': { $in: hashes.map(hash => Buffer.from(hash)) } });
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
		return collection.aggregate([conditions, grouping], { promoteLongs: false })
			.sort({ _id: -1 })
			.toArray()
			.then(this.sanitizer.deleteIds);
	}

	// region account retrieval

	/**
	 * Retrieves filtered and paginated accounts
	 * @param {Uint8Array} address Filters by address
	 * @param {uint64} mosaicId Filters by accounts with some mosaicId balance. Required if provided `sortField` is `balance`
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<object>} Accounts page.
	 */
	accounts(address, mosaicId, options) {
		const sortingOptions = { id: '_id', balance: 'account.mosaics.amount' };
		let conditions = {};

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		if (undefined !== address)
			conditions['account.address'] = Buffer.from(address);

		if (undefined !== mosaicId)
			conditions['account.mosaics.id'] = convertToLong(mosaicId);

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };

		let queryPromise;
		if ('balance' === options.sortField) {
			const { pageSize } = options;
			const pageIndex = options.pageNumber - 1;

			// fetch result sorted by specific mosaic amount, this unwinds mosaics and only returns matching mosaics (incomplete response)
			queryPromise = this.database.collection('accounts')
				.aggregate([], { promoteLongs: false })
				.unwind('$account.mosaics')
				.match(conditions)
				.sort(sortConditions)
				.skip(pageSize * pageIndex)
				.limit(pageSize)
				.toArray()
				.then(accounts => {
					const accountIds = accounts.map(account => account._id);
					const newConditions = { _id: { $in: accountIds } };
					// repeat the response with the found and sorted account ids, so that the result can be complete with all the mosaics
					// Second query set pageIndex to 0;
					return this.queryPagedDocuments(newConditions, [], {}, 'accounts', { pageSize, pageNumber: 1 })
						.then(fullAccountsPage => {
							// $in results do not preserve query order
							fullAccountsPage.data.sort((account1, account2) =>
								accountIds.findIndex(accountId => accountId.equals(account1.id))
								- accountIds.findIndex(accountId => accountId.equals(account2.id)));
							fullAccountsPage.pagination.pageNumber = options.pageNumber;
							return fullAccountsPage;
						});
				});
		} else {
			queryPromise = this.queryPagedDocuments(conditions, [], sortConditions, 'accounts', options);
		}

		return queryPromise.then(accountsPage => {
			accountsPage.data.map(pickTopImportance);
			return accountsPage;
		});
	}

	accountsByIds(ids) {
		// id will either have address property or publicKey property set; in the case of publicKey, convert it to address
		const buffers = ids.map(id => Buffer.from((id.publicKey
			? catapult.model.address.publicKeyToAddress(id.publicKey, this.networkId) : id.address)));
		return this.database.collection('accounts')
			.find({ 'account.address': { $in: buffers } })
			.toArray()
			.then(this.sanitizer.renameIds)
			.then(entities => entities.map(pickTopImportance));
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
