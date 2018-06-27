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

const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');
const connector = require('./connector');

const { address, EntityType } = catapult.model;
const { ObjectId } = MongoDb;

const isAggregateType = document => EntityType.aggregateComplete === document.transaction.type
	|| EntityType.aggregateBonded === document.transaction.type;

const createLong = value => {
	if (Number.isInteger(value))
		return MongoDb.Long.fromNumber(value);

	// if value is an array, assume it is a uint64
	return Array.isArray(value) ? new MongoDb.Long(value[0], value[1]) : value;
};

const createAccountTransactionsAllConditions = (publicKey, networkId) => {
	const decodedAddress = address.publicKeyToAddress(publicKey, networkId);
	const bufferPublicKey = Buffer.from(publicKey);
	const bufferAddress = Buffer.from(decodedAddress);
	return {
		$or: [
			{ 'transaction.cosignatures.signer': bufferPublicKey },
			{ 'meta.addresses': bufferAddress }
		]
	};
};

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
	const one = createLong(1);
	const startHeight = height.isZero() ? chainHeight.subtract(numBlocks).add(one) : height;

	// In all cases endHeight is actually max height + 1.
	const calculatedEndHeight = startHeight.add(numBlocks);
	const chainEndHeight = chainHeight.add(one);

	const endHeight = calculatedEndHeight.lessThan(chainEndHeight) ? calculatedEndHeight : chainEndHeight;
	return { startHeight, endHeight, numBlocks: endHeight.subtract(startHeight).toNumber() };
};

const boundPageSize = (pageSize, bounds) => Math.max(bounds.pageSizeMin, Math.min(bounds.pageSizeMax, pageSize));

class CatapultDb {
	// region construction / connect / disconnect

	constructor(options) {
		this.networkId = options.networkId;
		if (!this.networkId)
			throw Error('network id is required');

		this.pageSizeMin = options.pageSizeMin || 10;
		this.pageSizeMax = options.pageSizeMax || 100;
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
			return;

		this.client.close();
		this.client = undefined;
		this.database = undefined;
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
			.then(this.sanitizer.copyAndDeleteIds);
	}

	queryPagedDocuments(collectionName, conditions, id, pageSize, options = {}) {
		const sortOrder = options.sortOrder || -1;
		if (id)
			conditions.$and.push({ _id: { [0 > sortOrder ? '$lt' : '$gt']: new ObjectId(id) } });

		const collection = this.database.collection(collectionName);
		return collection.find(conditions)
			.project(options.projection)
			.sort({ _id: sortOrder })
			.limit(boundPageSize(pageSize, this))
			.toArray();
	}

	// endregion

	// region retrieval

	/**
	 * Retrieves sizes of database collections.
	 * @returns {Promise} Promise that resolves to the sizes of collections in the database.
	 */
	storageInfo() {
		const blockCountPromise = this.database.collection('blocks').count();
		const transactionCountPromise = this.database.collection('transactions').count();
		const accountCountPromise = this.database.collection('accounts').count();
		return Promise.all([blockCountPromise, transactionCountPromise, accountCountPromise])
			.then(storageInfo => ({ numBlocks: storageInfo[0], numTransactions: storageInfo[1], numAccounts: storageInfo[2] }));
	}

	chainInfo() {
		return this.queryDocument('chainInfo', {}, { _id: 0 });
	}

	blockAtHeight(height) {
		return this.queryDocument('blocks', { 'block.height': createLong(height) }, { 'meta.merkleTree': 0 })
			.then(this.sanitizer.deleteId);
	}

	blockWithMerkleTreeAtHeight(height) {
		return this.queryDocument('blocks', { 'block.height': createLong(height) })
			.then(this.sanitizer.deleteId);
	}

	blocksFrom(height, numBlocks) {
		if (0 === numBlocks)
			return Promise.resolve([]);

		return this.chainInfo().then(chainInfo => {
			const blockCollection = this.database.collection('blocks');
			const options = buildBlocksFromOptions(createLong(height), createLong(numBlocks), chainInfo.height);

			return blockCollection.find({ 'block.height': { $gte: options.startHeight, $lt: options.endHeight } })
				.project({ 'meta.merkleTree': 0 })
				.sort({ 'block.height': -1 })
				.toArray()
				.then(this.sanitizer.deleteIds)
				.then(blocks => Promise.resolve(blocks));
		});
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

	transactionsAtHeight(height, id, pageSize) {
		return this.queryTransactions({ 'meta.height': createLong(height) }, id, pageSize, { sortOrder: 1 });
	}

	transactionsByIdsImpl(collectionName, conditions) {
		return this.queryDocumentsAndCopyIds(collectionName, conditions, { projection: { 'meta.addresses': 0 } })
			.then(documents => Promise.all(documents.map(document => {
				if (!document)
					return document;

				if (!isAggregateType(document))
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
	 * @param {*} transactionType The transaction type.
	 * @param {object} fieldNames The descriptor for fields used in query.
	 * @returns {Promise.<array>} The promise that is resolved when tuples are ready.
	 */
	findNamesByIds(ids, transactionType, fieldNames) {
		const queriedIds = ids.map(createLong);
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

	// region transaction retrieval for account

	accountTransactionsAll(publicKey, id, pageSize) {
		const conditions = createAccountTransactionsAllConditions(publicKey, this.networkId);
		return this.queryTransactions(conditions, id, pageSize);
	}

	accountTransactionsIncoming(publicKey, id, pageSize) {
		const decoded = address.publicKeyToAddress(publicKey, this.networkId);
		const bufferAddress = Buffer.from(decoded);
		return this.queryTransactions({ 'transaction.recipient': bufferAddress }, id, pageSize);
	}

	accountTransactionsOutgoing(publicKey, id, pageSize) {
		const bufferPublicKey = Buffer.from(publicKey);
		return this.queryTransactions({ 'transaction.signer': bufferPublicKey }, id, pageSize);
	}

	accountTransactionsUnconfirmed(publicKey, id, pageSize) {
		const conditions = createAccountTransactionsAllConditions(publicKey, this.networkId);
		return this.queryTransactions(conditions, id, pageSize, { collectionName: 'unconfirmedTransactions' });
	}

	accountTransactionsPartial(publicKey, id, pageSize) {
		const conditions = createAccountTransactionsAllConditions(publicKey, this.networkId);
		return this.queryTransactions(conditions, id, pageSize, { collectionName: 'partialTransactions' });
	}

	// endregion

	// region account retrieval

	accountsByIds(ids) {
		// id will either have address property or publicKey property set; in the case of publicKey, convert it to address
		const buffers = ids.map(id => Buffer.from((id.publicKey ? address.publicKeyToAddress(id.publicKey, this.networkId) : id.address)));
		return this.queryDocuments('accounts', { 'account.address': { $in: buffers } })
			.then(entities => entities.map(accountWithMetadata => {
				const { account } = accountWithMetadata;
				if (0 < account.importances.length) {
					const importanceSnapshot = account.importances.pop();
					account.importance = importanceSnapshot.value;
					account.importanceHeight = importanceSnapshot.height;
				} else {
					account.importance = createLong(0);
					account.importanceHeight = createLong(0);
				}

				delete account.importances;
				return accountWithMetadata;
			}));
	}

	// endregion

	// region failed transaction

	/**
	 * Retrieves transaction results for the given hashes.
	 * @param {Array.<Uint8Array>} hashes The transaction hashes.
	 * @returns {Promise.<Array>} The promise that resolves to the array of hash / validation result pairs.
	 */
	transactionsByHashesFailed(hashes) {
		const buffers = hashes.map(hash => Buffer.from(hash));
		return this.queryDocuments('transactionStatuses', { hash: { $in: buffers } });
	}

	// endregion
}

module.exports = CatapultDb;
