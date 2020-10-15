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

const testDbOptions = require('./testDbOptions');
const CatapultDb = require('../../../src/db/CatapultDb');
const test = require('../../testUtils');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { address } = catapult.model;

const { Binary, Long, ObjectId } = MongoDb;

const Default_Height = 34567;
const Key_Size = 32;

const DefaultPagingOptions = {
	pageSizeMin: 10,
	pageSizeMax: 100,
	pageSizeDefault: 20
};

const createDbCollection = (db, collectionName) =>
	// note: db.database.collection() will always return a collection even if it doesn't exist in the database
	db.database.collection(collectionName)
		.drop()
		.catch(() => Promise.resolve());

const createObjectId = id => new ObjectId(`${'00'.repeat(12)}${id.toString(16)}`.slice(-24));

const createMosaics = count => {
	const mosaics = [];
	for (let i = 0; i < count; ++i)
		mosaics.push({ id: Long.fromNumber(i), amount: Long.fromNumber(i * i) });

	return mosaics;
};

const createImportances = count => {
	const importances = [];
	for (let i = 1; i <= count; ++i)
		importances.push({ value: Long.fromNumber(i), height: Long.fromNumber(i * i) });

	return importances.reverse();
};

const createAccount = (objectId, publicKey, savePublicKey, mosaics, importances) => {
	const decoded = Buffer.from(address.publicKeyToAddress(publicKey, testDbOptions.networkId));
	const account = {
		address: new Binary(decoded),
		addressHeight: Long.fromNumber(123),
		importances,
		mosaics
	};
	if (savePublicKey) {
		account.publicKey = new Binary(publicKey);
		account.publicKeyHeight = Long.fromNumber(234);
	} else {
		// use zeroed public key so the account size stays the same if later the real public key is stored
		account.publicKey = new Binary(Buffer.alloc(Key_Size, 0));
		account.publicKeyHeight = Long.fromNumber(0);
	}

	return { _id: createObjectId(objectId), account };
};

const createAccounts = (publicKey, options) => {
	options.numAccounts = undefined === options.numAccounts ? 10 : options.numAccounts;
	options.numImportances = undefined === options.numImportances ? 3 : options.numImportances;
	const accounts = [];

	// note: the first account in the array is not random since it is used in tests
	accounts.push(createAccount(
		1,
		publicKey,
		options.savePublicKey,
		createMosaics(options.numMosaics),
		createImportances(options.numImportances)
	));

	for (let i = 0; i < options.numAccounts; ++i) {
		accounts.push(createAccount(
			i + 2,
			test.random.publicKey(),
			0 === i % 2 || options.saveRandomPublicKey,
			createMosaics(options.numMosaics),
			createImportances(options.numImportances)
		));
	}

	return accounts;
};

const createDbBlock = height => {
	// meta data
	const meta = {
		hash: new Binary(test.random.hash()),
		generationHash: new Binary(test.random.hash()),
		totalFee: Long.fromNumber(12345),
		transactionsCount: 5,
		statementsCount: 5,
		transactionMerkleTree: [new Binary(test.random.hash()), new Binary(test.random.hash())],
		statementMerkleTree: [new Binary(test.random.hash()), new Binary(test.random.hash())]
	};

	// block header data
	const block = {
		signature: new Binary(test.random.signature()),
		signerPublicKey: new Binary(test.random.publicKey()),
		version: 234,
		type: 345,
		timestamp: Long.fromNumber(23456),
		height: Long.fromNumber(height),
		difficulty: Long.fromNumber(45678),
		previousBlockHash: new Binary(test.random.hash()),
		transactionsHash: new Binary(test.random.hash())
	};

	return { meta, block };
};

const createDbBlockWithId = (id, height) => Object.assign({ _id: createObjectId(id) }, createDbBlock(height));

const createDbTransaction = (id, signerPublicKey, recipientAddress, options) => {
	// meta data
	const meta = {
		hash: new Binary(test.random.hash()),
		height: Long.fromNumber((options || {}).height || Default_Height),
		addresses: []
	};

	// transaction data
	const transaction = {
		signature: new Binary(test.random.signature()),
		signerPublicKey: new Binary(signerPublicKey),
		version: 432,
		type: 543,
		timestamp: Long.fromNumber(65432),
		maxFee: Long.fromNumber(76543),
		deadline: Long.fromNumber(87654),
		recipientAddress: new Binary(recipientAddress),
		message: { size: 12, payload: new Binary(test.random.bytes(12)) },
		mosaics: []
	};
	for (let j = 0; 3 < j; ++j)
		transaction.mosaics.push({ id: Long.fromNumber(j), amount: Long.fromNumber(j * j) });

	return { _id: id, meta, transaction };
};

const createDbTransactions = (numRounds, signerPublicKey, recipientAddress) => {
	// Each round consists of
	// - 1 random transaction
	// - 1 transaction with signerPublicKey
	// - 1 random transaction
	// - 1 transaction with recipient
	// - 1 random transaction
	// - 1 transaction with signerPublicKey and recipient address
	// - 1 random transaction with aggregateId
	// - 1 transaction with signerPublicKey and aggregateId
	// all in all we have 8 * numRounds transactions
	let id = 0;
	const transactions = [];

	const push = (txSigner, txRecipient) => {
		transactions.push(createDbTransaction(createObjectId(++id), txSigner, txRecipient));
	};

	for (let i = 0; i < numRounds; ++i) {
		push(test.random.publicKey(), test.random.address());
		push(signerPublicKey, test.random.address());
		push(test.random.publicKey(), test.random.address());
		push(test.random.publicKey(), recipientAddress);
		push(test.random.publicKey(), test.random.address());
		push(signerPublicKey, recipientAddress);
		push(test.random.publicKey(), test.random.address());
		transactions[transactions.length - 1].meta.aggregateId = createObjectId(id);
		push(signerPublicKey, test.random.address());
		transactions[transactions.length - 1].meta.aggregateId = createObjectId(id);
	}

	return transactions;
};

const createChainStatistic = (height, scorelow, scoreHigh) => ({
	current: {
		height: Long.fromNumber(height),
		scoreLow: Long.fromNumber(scorelow),
		scoreHigh: Long.fromNumber(scoreHigh)
	}
});

const collectionUtils = {
	names: ['blocks', 'transactions', 'unconfirmedTransactions', 'partialTransactions',
		'transactionStatuses', 'accounts', 'chainStatistic', 'finalizedBlocks'],
	findInEntities: (dbEntities, collectionName) => {
		if ('blocks' !== collectionName)
			return dbEntities[collectionName];

		return 'blocks' in dbEntities ? dbEntities.blocks : dbEntities.block;
	}
};

const populateCollection = (db, collectionName, seed) => createDbCollection(db, collectionName).then(() => {
	if (seed) {
		const insertionFunction = Array.isArray(seed) ? 'insertMany' : 'insertOne';
		return db.database.collection(collectionName)[insertionFunction](seed);
	}

	return Promise.resolve();
});

const populateDatabase = (db, dbEntities) => {
	const promises = [];
	collectionUtils.names.forEach(collectionName => {
		const seed = collectionUtils.findInEntities(dbEntities, collectionName);
		const createCollectionPromise = populateCollection(db, collectionName, seed);
		promises.push(createCollectionPromise);
	});

	return Promise.all(promises);
};

const copyAndDeleteId = (item, collectionName) => {
	if (!['accounts', 'blocks'].includes(collectionName) && item.meta)
		item.meta.id = item._id;

	delete item._id;
};

const sanitizeDbEntities = (collectionName, seed) => {
	if (Array.isArray(seed))
		seed.forEach(item => copyAndDeleteId(item, collectionName));
	else if (seed)
		copyAndDeleteId(seed, collectionName);
};

const runDbTest = (dbEntities, collectionName, createDbFacade, issueDbCommand, assertDbCommandResult) => {
	// Arrange:
	const db = new CatapultDb(Object.assign({ networkId: testDbOptions.networkId }, DefaultPagingOptions));
	const dbFacade = createDbFacade(db);

	// Act + Assert:
	return db.connect(testDbOptions.url, 'test')
		.then(() => populateCollection(db, collectionName, dbEntities))
		.then(() => sanitizeDbEntities(collectionName, dbEntities))
		.then(() => issueDbCommand(dbFacade))
		.then(assertDbCommandResult)
		.then(() => db.close());
};

const dbTestUtils = {
	collection: collectionUtils,
	db: {
		createDbCollection,
		createObjectId,
		createAccount,
		createAccounts,
		createDbBlock,
		createDbBlockWithId,
		createDbTransaction,
		createDbTransactions,
		createChainStatistic,
		populateCollection,
		populateDatabase,
		sanitizeDbEntities,
		runDbTest
	}
};
Object.assign(dbTestUtils, test);

module.exports = dbTestUtils;
