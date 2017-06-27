/** @module db/CatapultDb */

import catapult from 'catapult-sdk';
import MongoDb from 'mongodb';
import connector from './connector';

const address = catapult.model.address;
const aggregateEntityType = catapult.model.EntityType.aggregate;

const ObjectId = MongoDb.ObjectID;
const Binary = MongoDb.Binary;

function createLong(value) {
	if (Number.isInteger(value))
		return MongoDb.Long.fromNumber(value);

	// if value is an array, assume it is a uint64
	return Array.isArray(value) ? new MongoDb.Long(value[0], value[1]) : value;
}

function createAccountTransactionsAllConditions(publicKey, networkId) {
	const decoded = address.publicKeyToAddress(publicKey, networkId);
	const bufferPublicKey = Buffer.from(publicKey);
	const bufferAddress = Buffer.from(decoded);
	return {
		$or: [
			{ 'transaction.signer': bufferPublicKey },
			{ 'transaction.recipient': bufferAddress },
			{ 'transaction.cosignatures': { $elemMatch: { signer: bufferPublicKey } } }
		]
	};
}

function createSanitizer() {
	return {
		copyAndDeleteId: dbObject => {
			if (dbObject) {
				dbObject.meta.id = dbObject._id;
				delete dbObject._id;
			}

			return dbObject;
		},

		copyAndDeleteIds: dbObjects => {
			for (const dbObject of dbObjects) {
				dbObject.meta.id = dbObject._id;
				delete dbObject._id;
			}

			return dbObjects;
		},

		deleteId: dbObject => {
			if (dbObject)
				delete dbObject._id;

			return dbObject;
		},

		deleteIds: dbObjects => {
			for (const dbObject of dbObjects)
				delete dbObject._id;

			return dbObjects;
		}
	};
}

function mapToPromise(dbObject) {
	return Promise.resolve(null === dbObject ? undefined : dbObject);
}

function buildBlocksFromOptions(height, numBlocks, chainHeight) {
	const one = createLong(1);
	const startHeight = height.isZero() ? chainHeight.subtract(numBlocks).add(one) : height;

	// In all cases endHeight is actually max height + 1.
	const calculatedEndHeight = startHeight.add(numBlocks);
	const chainEndHeight = chainHeight.add(one);

	const endHeight = calculatedEndHeight.lessThan(chainEndHeight) ? calculatedEndHeight : chainEndHeight;
	return { startHeight, endHeight, numBlocks: endHeight.subtract(startHeight).toNumber() };
}

function boundPageSize(pageSize, bounds) {
	return Math.max(bounds.pageSizeMin, Math.min(bounds.pageSizeMax, pageSize));
}

export default class CatapultDb {

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
			.then(db => { this.database = db; });
	}

	close() {
		if (!this.database)
			return;

		this.database.close();
		this.database = undefined;
	}

	// endregion

	// region helpers

	queryDocument(collectionName, conditions, fields) {
		const collection = this.database.collection(collectionName);
		return collection.findOne(conditions, fields)
			.then(mapToPromise);
	}

	queryPagedDocuments(collectionName, conditions, id, pageSize, options) {
		const sortOrder = (options || {}).sortOrder || -1;
		if (id)
			conditions.$and.push({ _id: { [0 > sortOrder ? '$lt' : '$gt']: new ObjectId(id) } });

		const collection = this.database.collection(collectionName);
		return collection.find(conditions)
			.sort({ _id: sortOrder })
			.limit(boundPageSize(pageSize, this))
			.toArray()
			.then(entities => Promise.resolve(entities));
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
				.sort({ 'block.height': -1 })
				.toArray()
				.then(this.sanitizer.deleteIds)
				.then(blocks => Promise.resolve(blocks));
		});
	}

	queryDependentDocuments(collectionName, aggregateIds) {
		if (0 === aggregateIds.length)
			return Promise.resolve([]);

		const collection = this.database.collection(collectionName);
		return collection.find({ 'meta.aggregateId': { $in: aggregateIds } })
			.toArray()
			.then(this.sanitizer.copyAndDeleteIds);
	}

	queryTransactions(conditions, id, pageSize, options) {
		// filter out dependent documents
		const collectionName = (options || {}).collectionName || 'transactions';
		const transactionConditions = { $and: [{ 'meta.aggregateId': { $exists: false } }, conditions] };
		return this.queryPagedDocuments(collectionName, transactionConditions, id, pageSize, options)
			.then(this.sanitizer.copyAndDeleteIds)
			.then(transactions => {
				const aggregateIds = [];
				const aggregateIdToTransactionMap = {};
				for (const document of transactions) {
					if (aggregateEntityType === document.transaction.type) {
						const aggregateId = document.meta.id;
						aggregateIds.push(aggregateId);
						aggregateIdToTransactionMap[aggregateId.toString()] = document.transaction;
					}
				}

				return this.queryDependentDocuments(collectionName, aggregateIds).then(dependentDocuments => {
					for (const document of dependentDocuments) {
						const transaction = aggregateIdToTransactionMap[document.meta.aggregateId];
						if (!transaction.transactions)
							transaction.transactions = [];

						transaction.transactions.push(document);
					}

					return transactions;
				});
			});
	}

	transactionsAtHeight(height, id, pageSize) {
		return this.queryTransactions({ 'meta.height': createLong(height) }, id, pageSize, { sortOrder: 1 });
	}

	transactionById(id) {
		return this.queryDocument('transactions', { _id: new ObjectId(id) })
			.then(this.sanitizer.copyAndDeleteId);
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
			.then(this.sanitizer.deleteIds)
			.then(entities => Promise.resolve(entities));
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

	// endregion

	// region account retrieval

	accountGet(decodedAddress) {
		const decoded = Buffer.from(decodedAddress);
		const bufferAddress = new Binary(decoded, 0);
		const accountCollection = this.database.collection('accounts');
		return accountCollection.findOne({ 'account.address': bufferAddress }, { _id: 0 })
			.then(accountWithMetaData => {
				if (null === accountWithMetaData)
					return undefined;

				const account = accountWithMetaData.account;
				if (0 < account.importances.length) {
					const importanceSnapshot = account.importances.pop();
					account.importance = importanceSnapshot.value;
					account.importanceHeight = importanceSnapshot.height;
				} else {
					account.importance = createLong(0);
					account.importanceHeight = createLong(0);
				}

				delete account.importances;
				return accountWithMetaData;
			});
	}

	accountGetFromPublicKey(publicKey) {
		const decodedAddress = address.publicKeyToAddress(publicKey, this.networkId);
		return this.accountGet(decodedAddress);
	}

	// endregion
}
