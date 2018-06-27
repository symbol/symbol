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

const { expect } = require('chai');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');
const CatapultDb = require('../../src/db/CatapultDb');
const test = require('./utils/dbTestUtils');
const testDbOptions = require('./utils/testDbOptions');

const { address, EntityType } = catapult.model;

const { Long, Binary } = MongoDb;
const Mijin_Test_Network = testDbOptions.networkId;
const Default_Height = 34567;

describe('catapult db', () => {
	const deleteIds = dbEntities => {
		test.collection.names.forEach(collectionName => {
			const seed = test.collection.findInEntities(dbEntities, collectionName);
			test.db.sanitizeDbEntities(collectionName, seed);
		});
	};

	const keyToAddress = key => Buffer.from(address.publicKeyToAddress(key, Mijin_Test_Network));

	const runDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) => {
		// Arrange:
		const db = new CatapultDb({ networkId: Mijin_Test_Network });

		// Act + Assert:
		return db.connect(testDbOptions.url, 'test')
			.then(() => test.db.populateDatabase(db, dbEntities))
			.then(() => deleteIds(dbEntities))
			.then(() => issueDbCommand(db))
			.then(assertDbCommandResult)
			.then(() => db.close());
	};

	const assertEqualDocuments = (expectedDocuments, actualDocuments) => {
		const stripPrivateInformation = transactions => transactions.map(transaction => {
			// addresses metadata is not exposed outside of the database class
			const modifiedTransaction = Object.assign({}, transaction);
			delete modifiedTransaction.meta.addresses;
			return modifiedTransaction;
		});

		const getAttributes = documents => {
			const documentToIdString = document => (document && document.meta ? document.meta.id.toString() : undefined);
			const subTxIds = documents.map(document => (document.transaction.transactions || []).map(documentToIdString));
			return {
				numDocuments: documents.length,
				ids: documents.map(documentToIdString),
				numSubTxes: subTxIds.reduce((sum, ids) => sum + ids.length, 0),
				subTxIds
			};
		};

		// clean transaction data
		const sanitizedExpectedDocuments = expectedDocuments[0] && expectedDocuments[0].transaction
			? stripPrivateInformation(expectedDocuments)
			: expectedDocuments;

		const expectedAttributes = getAttributes(sanitizedExpectedDocuments);
		const actualAttributes = getAttributes(actualDocuments);

		// Assert:
		expect(actualAttributes.numDocuments, 'wrong number of documents').to.equal(expectedAttributes.numDocuments);
		expect(actualAttributes.ids, 'wrong ids').to.deep.equal(expectedAttributes.ids);
		expect(actualAttributes.numSubTxes, 'wrong number of sub documents').to.equal(expectedAttributes.numSubTxes);
		expect(actualAttributes.subTxIds, 'wrong sub document ids').to.deep.equal(expectedAttributes.subTxIds);
		expect(actualDocuments).to.deep.equal(expectedDocuments);
	};

	describe('basic', () => {
		it('cannot create db without network id', () => {
			// Act + Assert:
			expect(() => new CatapultDb({})).to.throw('network id is required');
		});

		it('can close unconnected db', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: Mijin_Test_Network });

			// Act + Assert: no exception
			expect(() => db.close()).to.not.throw();
		});
	});

	describe('storage info', () => {
		it('can retrieve storage info', () => {
			const Rounds = 2;

			// Arrange:
			return runDbTest(
				{
					block: test.db.createDbBlock(),
					transactions: test.db.createDbTransactions(Rounds, test.random.publicKey(), test.random.address())
				},
				db => db.storageInfo(),
				storageInfo => expect(storageInfo).to.deep.equal({ numBlocks: 1, numTransactions: Rounds * 8, numAccounts: 0 })
			);
		});
	});

	describe('chain info', () => {
		it('can retrieve chain info', () =>
			// Assert:
			runDbTest(
				{ chainInfo: test.db.createChainInfo(1357, 2468, 3579) },
				db => db.chainInfo(),
				chainInfo => expect(chainInfo).to.deep.equal(test.db.createChainInfo(1357, 2468, 3579))
			));
	});

	const stripPrivateBlockInformation = block => {
		// block merkle tree is not exposed outside of the database class unless explicitly requested
		const modifiedBlock = Object.assign({}, block);
		delete modifiedBlock.meta.merkleTree;
		return modifiedBlock;
	};

	describe('block at height', () => {
		it('undefined is returned for block at unknown height', () =>
			// Assert:
			runDbTest(
				{ block: test.db.createDbBlock(Default_Height) },
				db => db.blockAtHeight(Long.fromNumber(Default_Height + 1)),
				block => expect(block).to.equal(undefined)
			));

		// use blockAtHeight tests as a proxy for testing support of different numeric types (Number, uint64, Long)

		const assertCanRetrieveSimpleBlock = height => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);

			// Assert:
			return runDbTest(
				{ block: seedBlock },
				db => db.blockAtHeight(height),
				block => expect(block).to.deep.equal(stripPrivateBlockInformation(seedBlock))
			);
		};

		it('can retrieve block without transactions at height (Number)', () => assertCanRetrieveSimpleBlock(Default_Height));
		it('can retrieve block without transactions at height (uint64)', () => assertCanRetrieveSimpleBlock([Default_Height, 0]));
		it('can retrieve block without transactions at height (Long)', () => assertCanRetrieveSimpleBlock(Long.fromNumber(Default_Height)));

		it('can retrieve block with transactions at height', () => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);
			const blockTransactions = test.db.createDbTransactions(2, test.random.publicKey(), test.random.address());

			// Assert:
			return runDbTest(
				{ block: seedBlock, transactions: blockTransactions },
				db => db.blockAtHeight(Long.fromNumber(Default_Height)),
				block => expect(block).to.deep.equal(stripPrivateBlockInformation(seedBlock))
			);
		});
	});

	describe('block at height with merkle tree', () => {
		it('undefined is returned for block at unknown height', () =>
			// Assert:
			runDbTest(
				{ block: test.db.createDbBlock(Default_Height) },
				db => db.blockWithMerkleTreeAtHeight(Long.fromNumber(Default_Height + 1)),
				block => expect(block).to.equal(undefined)
			));

		// use blockAtHeight tests as a proxy for testing support of different numeric types (Number, uint64, Long)

		const assertCanRetrieveSimpleBlock = height => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);

			// Assert:
			return runDbTest(
				{ block: seedBlock },
				db => db.blockWithMerkleTreeAtHeight(height),
				block => expect(block).to.deep.equal(seedBlock)
			);
		};

		it('can retrieve block with merkle tree at height (Number)', () => assertCanRetrieveSimpleBlock(Default_Height));
		it('can retrieve block with merkle tree at height (uint64)', () => assertCanRetrieveSimpleBlock([Default_Height, 0]));
		it('can retrieve block with merkle tree at height (Long)', () => assertCanRetrieveSimpleBlock(Long.fromNumber(Default_Height)));

		it('can retrieve block with merkle tree at height', () => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);
			const blockTransactions = test.db.createDbTransactions(2, test.random.publicKey(), test.random.address());

			// Assert:
			return runDbTest(
				{ block: seedBlock, transactions: blockTransactions },
				db => db.blockWithMerkleTreeAtHeight(Long.fromNumber(Default_Height)),
				block => expect(block).to.deep.equal(seedBlock)
			);
		});
	});

	describe('blocks from height', () => {
		const createBlocks = (height, numBlocks) => {
			const blocks = [];
			// the blocks are created in descending height order
			// in order to make later comparisons easier using .slice()
			for (let i = 0; i < numBlocks; ++i)
				blocks.push(test.db.createDbBlock(height + numBlocks - 1 - i));

			return blocks;
		};

		const createDbEntities = numBlocks => ({
			chainInfo: test.db.createChainInfo(Default_Height + numBlocks - 1, 0, 0),
			blocks: createBlocks(Default_Height, numBlocks)
		});

		it('returns empty array for unknown height', () =>
			// Assert:
			runDbTest(
				createDbEntities(1),
				db => db.blocksFrom(Long.fromNumber(Default_Height + 1), 10),
				blocks => expect(blocks).to.deep.equal([])
			));

		const assertBlocks = (actualBlocks, dbEntities, startHeight, numBlocks) => {
			// Assert: actual blocks should contain `numBlocks` blocks from `startHeight`.
			// dbEntities.blocks are sorted in descending order, so:
			// 1. inside the expected list find the index of element with height `startHeight`,
			// 2. go back numBlocks elements to find element with largest height.
			const endElement = dbEntities.blocks.findIndex(entity => entity.block.height.toNumber() === startHeight) + 1;
			const startElement = endElement - numBlocks;
			expect(actualBlocks.length).to.equal(numBlocks);
			expect(actualBlocks).to.deep.equal(dbEntities.blocks.slice(startElement, endElement).map(stripPrivateBlockInformation));
		};

		it('returns at most available blocks', () => {
			// Arrange:
			const dbEntities = createDbEntities(5);

			// Assert:
			return runDbTest(
				dbEntities,
				db => db.blocksFrom(Default_Height + 2, 10),
				blocks => assertBlocks(blocks, dbEntities, Default_Height + 2, 3)
			);
		});

		it('respects requested number of blocks', () => {
			// Arrange:
			const dbEntities = createDbEntities(10);

			// Assert:
			return runDbTest(
				dbEntities,
				db => db.blocksFrom(Default_Height + 4, 3),
				blocks => assertBlocks(blocks, dbEntities, Default_Height + 4, 3)
			);
		});

		it('returns empty array when requesting 0 blocks', () =>
			// Arrange:
			runDbTest(
				createDbEntities(10),
				db => db.blocksFrom(Default_Height + 4, 0),
				blocks => expect(blocks).to.deep.equal([])
			));

		it('returns top blocks when requesting from "0" height', () => {
			// Arrange:
			const dbEntities = createDbEntities(34);

			// Assert:
			return runDbTest(
				dbEntities,
				db => db.blocksFrom(0, 10),
				blocks => assertBlocks(blocks, dbEntities, Default_Height + 24, 10)
			);
		});

		it('returns top blocks when requesting from "0" height even if more blocks are in the database', () => {
			// Arrange: set height to 4
			const dbEntities = {
				chainInfo: test.db.createChainInfo(4, 0, 0),
				blocks: createBlocks(1, 10)
			};

			// Assert:
			return runDbTest(
				dbEntities,
				db => db.blocksFrom(0, 10),
				blocks => assertBlocks(blocks, dbEntities, 1, 4)
			);
		});

		it('returns last block when requesting from "0" height with alignment 1', () => {
			// Arrange: set height to 134
			const dbEntities = {
				chainInfo: test.db.createChainInfo(134, 0, 0),
				blocks: createBlocks(120, 15)
			};

			// Assert:
			return runDbTest(
				dbEntities,
				db => db.blocksFrom(0, 1),
				blocks => assertBlocks(blocks, dbEntities, 134, 1)
			);
		});

		it('returns blocks sorted in descending order', () =>
			// Arrange: try to insert in random order
			runDbTest(
				{
					chainInfo: test.db.createChainInfo(Default_Height + 2, 0, 0),
					blocks: [
						test.db.createDbBlock(Default_Height + 2),
						test.db.createDbBlock(Default_Height),
						test.db.createDbBlock(Default_Height + 1)
					]
				},
				db => db.blocksFrom(Default_Height, 5),
				// Assert: blocks are returned in proper order - descending by height
				blocks => {
					expect(blocks.length).to.equal(3);
					for (let i = 0; 3 > i; ++i)
						expect(blocks[i].block.height).to.deep.equal(Long.fromNumber(Default_Height + 2 - i));
				}
			));
	});

	const createTransactionHash = id => catapult.utils.convert.hexToUint8(`${'00'.repeat(16)}${id.toString(16)}`.slice(-32));

	const createSeedTransactions = (numTransactionsPerHeight, heights, options) => {
		// notice that generated transactions only contain what is used by transactionsAtHeight for filtering (meta.height)
		let id = 1;
		const transactions = [];
		const addTransactionAtHeight = (height, type) => {
			const aggregateId = test.db.createObjectId(id);
			const hash = new Binary(Buffer.from(createTransactionHash(id++)));
			const meta = { height, hash, addresses: [] };
			transactions.push({ _id: aggregateId, meta, transaction: { type } });

			const numDependentDocuments = (options || {}).numDependentDocuments || 0;
			for (let j = 0; j < numDependentDocuments; ++j)
				transactions.push({ _id: test.db.createObjectId(id++), meta: { height, aggregateId }, transaction: {} });
		};

		for (let i = 0; i < numTransactionsPerHeight; ++i) {
			// transactions at height should work for both aggregates, so pick each type alteratively
			const type = 0 === i % 2 ? EntityType.aggregateComplete : EntityType.aggregateBonded;
			heights.forEach(height => { addTransactionAtHeight(height, type); });
		}

		return transactions;
	};

	describe('transactions at height', () => {
		const addTests = traits => {
			it('returns empty array for unknown height', () => {
				// Arrange: at heights 17, 25 and 36 - for each height create 3 transactions
				const seedTransactions = traits.createSeedTransactions(3);
				return runDbTest(
					{ transactions: seedTransactions },
					db => db.transactionsAtHeight(30),
					transactions => {
						// Assert: no transactions are available at height 30
						assertEqualDocuments([], transactions);
					}
				);
			});

			it('can retrieve all transactions at height', () => {
				// Arrange: at heights 17, 25 and 36 - for each height create 3 transactions
				const seedTransactions = traits.createSeedTransactions(3);
				return runDbTest(
					{ transactions: seedTransactions },
					db => db.transactionsAtHeight(25),
					transactions => {
						// Assert: all three transactions at height 25 are returned
						assertEqualDocuments(traits.getHeight25Transactions(seedTransactions, [0, 1, 2]), transactions);
					}
				);
			});

			it('query respects supplied id', () => {
				// Arrange: at heights 17, 25 and 36 - for each height create 3 transactions
				const seedTransactions = traits.createSeedTransactions(3);
				return runDbTest(
					{ transactions: seedTransactions },
					// Act: query passing the id of the second transaction
					db => db.transactionsAtHeight(25, traits.getHeight25Transactions(seedTransactions, [1])[0].meta.id.toString()),
					transactions => {
						// Assert: only the transactions after the passed id should be returned
						assertEqualDocuments(traits.getHeight25Transactions(seedTransactions, [2]), transactions);
					}
				);
			});

			const assertPageSize = (numTransactionsPerHeight, pageSize, numExpectedTransactions) => {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions(numTransactionsPerHeight);
				return runDbTest(
					{ transactions: seedTransactions },
					// Act: query with a custom page size
					db => db.transactionsAtHeight(25, undefined, pageSize),
					transactions => {
						// Assert: at most a single page was returned starting from the first transaction
						const expectedIndexes = [];
						for (let i = 0; i < numExpectedTransactions; ++i)
							expectedIndexes.push(i);

						assertEqualDocuments(traits.getHeight25Transactions(seedTransactions, expectedIndexes), transactions);
					}
				);
			};

			// minimum and maximum values are set in CatapultDb ctor
			it('query respects page size', () => assertPageSize(14, 12, 12));
			it('query ensures minimum page size', () => assertPageSize(14, 3, 10));
			it('query ensures maximum page size', () => assertPageSize(150, 125, 100));
		};

		describe('for transactions', () => {
			addTests({
				// at heights 17, 25 and 36 - for each height create X transactions
				// - 17: 0000, 0003, 0006
				// - 25: 0001, 0004, 0007
				// - 36: 0002, 0005, 0008
				createSeedTransactions: numTransactionsPerHeight => createSeedTransactions(numTransactionsPerHeight, [17, 25, 36]),

				getHeight25Transactions: (seedTransactions, indexes) => indexes.map(index => seedTransactions[1 + (3 * index)])
			});
		});

		describe('for transactions with dependent documents', () => {
			addTests({
				// at heights 25 and 36 - for each height create X transactions with 2 dependent documents each
				// - 17: 0000 (0001, 0002), 0009 (000A, 000B), 0012 (0013, 0014)
				// - 25: 0003 (0004, 0005), 000C (000D, 000E), 0015 (0016, 0017)
				// - 36: 0006 (0007, 0008), 000F (0010, 0011), 0018 (0019, 0020)
				createSeedTransactions: numTransactionsPerHeight =>
					createSeedTransactions(numTransactionsPerHeight, [17, 25, 36], { numDependentDocuments: 2 }),

				getHeight25Transactions: (seedTransactions, indexes) => indexes.map(index => {
					const startIndex = 3 + (9 * index);
					const stitchedAggregate = Object.assign({}, seedTransactions[startIndex]);
					stitchedAggregate.transaction.transactions = [seedTransactions[startIndex + 1], seedTransactions[startIndex + 2]];
					return stitchedAggregate;
				})
			});
		});
	});

	describe('transaction by id', () => {
		const addTestsWithId = (traits, idTraits) => {
			it('can retrieve each transaction by id', () => {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions();
				const allIds = traits.allIds.map(idTraits.convertToId);

				// Act + Assert:
				return runDbTest(
					{ [idTraits.collectionName]: seedTransactions },
					db => idTraits.transactionsByIds(db, allIds),
					transactions => assertEqualDocuments(traits.expected(seedTransactions, traits.allIds), transactions)
				);
			});

			it('can retrieve transaction using known id', () => {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions();
				const documentId = idTraits.convertToId(traits.validId);

				// Act + Assert:
				return runDbTest(
					{ [idTraits.collectionName]: seedTransactions },
					db => idTraits.transactionsByIds(db, [documentId]),
					transactions => assertEqualDocuments(traits.expected(seedTransactions, [traits.validId]), transactions)
				);
			});

			it('cannot retrieve transaction using unknown id', () => {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions();
				const documentId = idTraits.convertToId(traits.invalidId);

				// Act + Assert:
				return runDbTest(
					{ [idTraits.collectionName]: seedTransactions },
					db => idTraits.transactionsByIds(db, [documentId]),
					transactions => expect(transactions).to.deep.equal([])
				);
			});

			it('can retrieve only known transactions by id', () => {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions();
				const allIds = traits.allIds.map(idTraits.convertToId);
				// make a copy and insert invalid id in the middle
				const mixedIds = allIds.slice();
				mixedIds.splice(mixedIds.length / 2, 0, idTraits.convertToId(traits.invalidId));

				// Act + Assert:
				return runDbTest(
					{ [idTraits.collectionName]: seedTransactions },
					db => idTraits.transactionsByIds(db, mixedIds),
					transactions => assertEqualDocuments(traits.expected(seedTransactions, traits.allIds), transactions)
				);
			});
		};

		const addTests = traits => {
			describe('by object id', () =>
				addTestsWithId(traits, {
					convertToId: test.db.createObjectId,
					collectionName: 'transactions',
					transactionsByIds: (db, ids) => db.transactionsByIds(ids)
				}));

			describe('by transaction hash', () =>
				addTestsWithId(traits, {
					convertToId: createTransactionHash,
					collectionName: 'transactions',
					transactionsByIds: (db, ids) => db.transactionsByHashes(ids)
				}));

			describe('by transaction hash (unconfirmed)', () =>
				addTestsWithId(traits, {
					convertToId: createTransactionHash,
					collectionName: 'unconfirmedTransactions',
					transactionsByIds: (db, ids) => db.transactionsByHashesUnconfirmed(ids)
				}));

			describe('by transaction hash (partial)', () =>
				addTestsWithId(traits, {
					convertToId: createTransactionHash,
					collectionName: 'partialTransactions',
					transactionsByIds: (db, ids) => db.transactionsByHashesPartial(ids)
				}));
		};

		describe('for transactions', () => {
			addTests({
				createSeedTransactions: () => createSeedTransactions(3, [21, 34]),
				expected: (transactions, ids) => ids.map(id => transactions[id - 1]),
				allIds: [1, 2, 3, 4, 5, 6],
				validId: 2,
				invalidId: (3 * 2) + 1
			});
		});

		describe('for transactions with dependent documents', () => {
			addTests({
				// 1 (2, 3)  (height 21)
				// 4 (5, 6)  (height 34)
				// 7 (8, 9)  (height 21)
				// ...
				createSeedTransactions: () => createSeedTransactions(3, [21, 34], { numDependentDocuments: 2 }),
				expected: (transactions, ids) => ids.map(id => {
					const index = id - 1;
					const stitchedAggregate = Object.assign({}, transactions[index]);
					stitchedAggregate.transaction.transactions = [transactions[index + 1], transactions[index + 2]];
					return stitchedAggregate;
				}),
				allIds: [1, 4, 7, 10, 13, 16],
				// transaction with id 4 is an aggregate
				validId: 4,
				invalidId: (3 * 3 * 2) + 1
			});
		});

		describe('for a dependent document', () => {
			it('can retrieve dependent document by id', () => {
				// Arrange:
				// transaction with id 5 is a dependent document
				// 1 (2, 3)  (height 21)
				// 4 (5, 6)  (height 34)
				// ...
				const seedTransactions = createSeedTransactions(3, [21, 34], { numDependentDocuments: 2 });
				const documentId = test.db.createObjectId(5);

				// Act + Assert:
				return runDbTest(
					{ transactions: seedTransactions },
					db => db.transactionsByIds([documentId]),
					transactions => assertEqualDocuments([seedTransactions[4]], transactions)
				);
			});
		});
	});

	describe('names by ids', () => {
		const createDbMarkedTransaction = (id, parentId, markerId) => {
			// meta data
			const meta = {};

			// transaction data
			const transaction = {
				type: 0x12345,
				parentMarkerId: Long.fromNumber(parentId),
				markerId: Long.fromNumber(markerId),
				markerName: `marker-${markerId}`
			};

			return { _id: id, meta, transaction };
		};

		const createDbMarkedTransactions = (numTransactions, numRepetitions) => {
			const transactions = [];
			let id = 0;
			// create multiple (numRepetitions) transactions with same markerId, but different _id field.
			for (let i = 0; i < numRepetitions; ++i) {
				for (let j = 0; j < numTransactions; ++j)
					transactions.push(createDbMarkedTransaction(test.db.createObjectId(++id), 15000 + j, 20000 + j));
			}

			return transactions;
		};

		const createDbEntities = () => ({ transactions: createDbMarkedTransactions(12, 3) });

		const assertTransactions = (expectedTransactions, ids) => runDbTest(
			// Arrange: seed with transactions with markerIds: 20000 - 20011
			createDbEntities(),
			db => db.findNamesByIds(ids, 0x12345, { id: 'markerId', name: 'markerName', parentId: 'parentMarkerId' }),
			transactions => {
				// Assert:
				expect(transactions.length).to.equal(expectedTransactions.length);
				expect(transactions).to.deep.equal(expectedTransactions);
			}
		);

		const createExpected = (parentId, markerId) => ({
			markerId: Long.fromNumber(markerId),
			markerName: `marker-${markerId}`,
			parentMarkerId: Long.fromNumber(parentId)
		});

		it('returns empty array for unknown ids', () =>
			// Act + Assert: query for markerId outside seed range
			assertTransactions([], [[123, 456]]));

		it('returns single matching entry', () => {
			// Act + Assert: query for markerId in seed range (20000-20011)
			// note: there are multiple transactions with same markerId, so this also checks that only non-duplicates are returned
			const expected = [createExpected(15010, 20010)];

			return assertTransactions(expected, [[20010, 0]]);
		});

		it('returns multiple matching entries', () => {
			// Act + Assert: query for markerIds in seed range (20000-20011)
			// note: there are multiple transactions with same markerId, so this also checks that only non-duplicates are returned
			const expected = [createExpected(15008, 20008), createExpected(15005, 20005), createExpected(15003, 20003)];

			return assertTransactions(expected, [[20003, 0], [20005, 0], [20008, 0]]);
		});

		it('returns only matching entries', () => {
			// Act + Assert: query for markerId in seed range (20000-20011) and one outside of range
			// note: there are multiple transactions with same markerId, so this also checks that only non-duplicates are returned
			const expected = [createExpected(15008, 20008), createExpected(15003, 20003)];

			return assertTransactions(expected, [[20003, 0], [123, 456], [20008, 0]]);
		});
	});

	describe('account transactions', () => {
		const getCollectionName = traits => [traits.collectionName || 'transactions'];

		const createTransactionMetadata = transaction => ({
			addresses: [keyToAddress(transaction.signer.buffer), transaction.recipient.buffer]
		});

		const dbTransactionTraits = {
			incoming: { dbFunctionName: 'accountTransactionsIncoming' },
			outgoing: { dbFunctionName: 'accountTransactionsOutgoing' },
			all: { dbFunctionName: 'accountTransactionsAll' },
			unconfirmed: { dbFunctionName: 'accountTransactionsUnconfirmed', collectionName: 'unconfirmedTransactions' },
			partial: { dbFunctionName: 'accountTransactionsPartial', collectionName: 'partialTransactions' }
		};

		describe('can filter by', () => {
			// creates transactions for three accounts such that: A is sender, B is recipient, C is sender and recipient
			// [0001] A -> B, [0002] A -> C, [0003] C -> B, [0004] C -> C
			const createDirectionalTransactions = (key1, key2, key3) => {
				const keys = [key1, key2, key3];
				const addresses = keys.map(keyToAddress);

				const createTransactionTemplate = (signer, recipient) => ({
					signer: new Binary(signer), recipient: new Binary(recipient)
				});

				const transactionTemplates = [
					createTransactionTemplate(keys[0], addresses[1]),
					createTransactionTemplate(keys[0], addresses[2]),
					createTransactionTemplate(keys[2], addresses[1]),
					createTransactionTemplate(keys[2], addresses[2])
				];

				let id = 1;
				const transactions = [];
				transactionTemplates.forEach(transaction => {
					transactions.push({ _id: test.db.createObjectId(id++), meta: createTransactionMetadata(transaction), transaction });
				});

				return transactions;
			};

			const addTests = traits => {
				const addDirectionalTest = (keySelector, expectedTransactionIndexes) => {
					// Arrange:
					const keys = [test.random.publicKey(), test.random.publicKey(), test.random.publicKey()];
					const seedTransactions = createDirectionalTransactions(keys[0], keys[1], keys[2]);
					return runDbTest(
						{ [getCollectionName(traits)]: seedTransactions },
						// Act: retrieve transactions for an account
						db => db[traits.dbFunctionName](keySelector(keys)),
						transactions => {
							// Assert: expected transactions are available
							assertEqualDocuments(expectedTransactionIndexes.map(index => seedTransactions[index]), transactions);
						}
					);
				};

				it('for account without transactions', () =>
					addDirectionalTest(() => test.random.publicKey(), [])); // random

				it('for account with outgoing only transactions', () =>
					addDirectionalTest(keys => keys[0], traits.directional.outgoing)); // A

				it('for account with incoming only transactions', () =>
					addDirectionalTest(keys => keys[1], traits.directional.incoming)); // B

				it('for account with incoming and outgoing transactions', () =>
					addDirectionalTest(keys => keys[2], traits.directional.incomingAndOutgoing)); // C
			};

			describe('incoming', () => {
				addTests(Object.assign({
					directional: { outgoing: [], incoming: [2, 0], incomingAndOutgoing: [3, 1] }
				}, dbTransactionTraits.incoming));
			});

			describe('outgoing', () => {
				addTests(Object.assign({
					directional: { outgoing: [1, 0], incoming: [], incomingAndOutgoing: [3, 2] }
				}, dbTransactionTraits.outgoing));
			});

			['all', 'unconfirmed', 'partial'].forEach(key => {
				addTests(Object.assign({
					directional: { outgoing: [1, 0], incoming: [2, 0], incomingAndOutgoing: [3, 2, 1] }
				}, dbTransactionTraits[key]));
			});
		});

		describe('can page', () => {
			const addTransactions = (transactions, id, signer, recipient, numDependentDocuments) => {
				const aggregateId = test.db.createObjectId(id);
				const aggregateType = 0 === transactions.length % 2 ? EntityType.aggregateComplete : EntityType.aggregateBonded;
				const transaction = { type: aggregateType, signer: new Binary(signer), recipient: new Binary(recipient) };
				transactions.push({ _id: aggregateId, meta: createTransactionMetadata(transaction), transaction });

				for (let j = 0; j < numDependentDocuments; ++j)
					transactions.push({ _id: test.db.createObjectId(id + j + 1), meta: { aggregateId }, transaction: {} });
			};

			const createBilateralTransactions = (keys, options) => {
				const addresses = keys.map(keyToAddress);

				let id = 0;
				const transactions = [];
				keys.forEach(signer => addresses.forEach(recipient => {
					const numDependentDocuments = (options || {}).numDependentDocuments || 0;
					addTransactions(transactions, id, signer, recipient, numDependentDocuments);
					id += 1 + numDependentDocuments;
				}));

				return transactions;
			};

			const createAlternatingTransactions = (numMatchingTransactions, createMatchingSenderRecipientPair, options) => {
				const transactions = [];
				const randomKey = test.random.publicKey();
				const randomAddress = keyToAddress(test.random.publicKey());

				let id = 0;
				for (let i = 0; i < 2 * numMatchingTransactions; ++i) {
					// create a matching transaction every even iteration
					const senderRecipientPair = 0 === id % 2
						? createMatchingSenderRecipientPair()
						: { signer: randomKey, recipient: randomAddress };
					const numDependentDocuments = (options || {}).numDependentDocuments || 0;
					addTransactions(transactions, id, senderRecipientPair.signer, senderRecipientPair.recipient, numDependentDocuments);
					id += 1 + numDependentDocuments;
				}

				return transactions;
			};

			// helper functions for creating transactions for paging tests
			const createPagingSeedTransactionsFactory = {
				curryIncoming: options => (numMatchingTransactions, key) => {
					// incoming recipient should match
					const signer = test.random.publicKey();
					const recipient = keyToAddress(key);
					return createAlternatingTransactions(numMatchingTransactions, () => ({ signer, recipient }), options);
				},

				curryOutgoing: options => (numMatchingTransactions, key) => {
					// outgoing signer should match
					const recipient = keyToAddress(test.random.publicKey());
					return createAlternatingTransactions(numMatchingTransactions, () => ({ signer: key, recipient }), options);
				},

				curryAll: options => (numMatchingTransactions, key) => {
					// incoming recipient and/or outgoing signer should match, so alternate
					let i = 0;
					const recipient = keyToAddress(key);
					const randomSigner = test.random.publicKey();
					const randomRecipient = keyToAddress(test.random.publicKey());
					return createAlternatingTransactions(
						numMatchingTransactions,
						() => (0 === i++ % 2 ? { signer: key, recipient: randomRecipient } : { signer: randomSigner, recipient }),
						options
					);
				}
			};

			const addTests = traits => {
				it('can retrieve all matching transactions', () => {
					// Arrange:
					const keys = [test.random.publicKey(), test.random.publicKey(), test.random.publicKey()];
					const seedTransactions = traits.createSeedTransactions(keys);
					return runDbTest(
						{ [getCollectionName(traits)]: seedTransactions },
						// Act: retrieve transactions from the second account
						db => db[traits.dbFunctionName](keys[1]),
						transactions => {
							// Assert: only the transactions matching the second account were returned
							assertEqualDocuments(traits.getAccount2Transactions(seedTransactions), transactions);
						}
					);
				});

				it('query respects supplied id', () => {
					// Arrange:
					const keys = [test.random.publicKey(), test.random.publicKey(), test.random.publicKey()];
					const seedTransactions = traits.createSeedTransactions(keys);
					return runDbTest(
						{ [getCollectionName(traits)]: seedTransactions },
						// Act: query passing a custom id
						db => db[traits.dbFunctionName](keys[1], traits.startId),
						transactions => {
							// Assert: only the transactions after the passed id should be returned
							assertEqualDocuments(traits.getAccount2FilteredTransactions(seedTransactions, [2]), transactions);
						}
					);
				});

				const assertPageSize = (numMatchingTransactions, pageSize, numExpectedTransactions) => {
					// Arrange: generate transactions that alternate matching the filter; Y, N, Y, N ...
					const key = test.random.publicKey();
					const seedTransactions = traits.createPagingSeedTransactions(numMatchingTransactions, key);
					return runDbTest(
						{ [getCollectionName(traits)]: seedTransactions },
						// Act: query with a custom page size
						db => db[traits.dbFunctionName](key, undefined, pageSize),
						transactions => {
							// Assert: at most a single page was returned (subtract 1 because oids are 0-based)
							const expectedIndexes = [];
							for (let i = 0; i < numExpectedTransactions; ++i)
								expectedIndexes.push(((numMatchingTransactions - i - 1) * 2) * traits.pageIdStep);

							assertEqualDocuments(traits.curryFromIndexes(expectedIndexes)(seedTransactions), transactions);
						}
					);
				};

				// minimum and maximum values are set in CatapultDb ctor
				it('query respects page size', () => assertPageSize(14, 12, 12));
				it('query ensures minimum page size', () => assertPageSize(14, 3, 10));
				it('query ensures maximum page size', () => assertPageSize(150, 125, 100));
			};

			describe('transactions', () => {
				// create 3 x 3 transactions (second account [B] is used by all tests)
				// - 0000 A -> A, 0001 A -> B, 0002 A -> C
				// - 0003 B -> A, 0004 B -> B, 0005 B -> C
				// - 0006 C -> A, 0007 C -> B, 0008 C -> C
				const basicTraits = {
					createSeedTransactions: createBilateralTransactions,
					curryFromIndexes: indexes => seedTransactions => indexes.map(index => seedTransactions[index]),
					startId: test.db.createObjectId(4), // for id test, the id passed in as a filter
					pageIdStep: 1 // difference between consecutive ids
				};

				// region test cases

				describe('incoming', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([7, 4, 1]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([1]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryIncoming()
					}, basicTraits, dbTransactionTraits.incoming));
				});

				describe('outgoing', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([5, 4, 3]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([3]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryOutgoing()
					}, basicTraits, dbTransactionTraits.outgoing));
				});

				['all', 'unconfirmed', 'partial'].forEach(key => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([7, 5, 4, 3, 1]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([3, 1]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryAll()
					}, basicTraits, dbTransactionTraits[key]));
				});

				// endregion
			});

			describe('transactions with dependent documents', () => {
				// create 3 x 3 transactions (second account [B] is used by all tests)
				// - 0000 (0001, 0002) A -> A, 0003 (0004, 0005) A -> B, 0006 (0007, 0008) A -> C
				// - 0009 (000A, 000B) B -> A, 000C (000D, 000E) B -> B, 000F (0010, 0011) B -> C
				// - 0012 (0013, 0014) C -> A, 0015 (0016, 0017) C -> B, 0018 (0019, 001A) C -> C
				const basicTraits = {
					createSeedTransactions: keys => createBilateralTransactions(keys, { numDependentDocuments: 2 }),
					curryFromIndexes: indexes => seedTransactions => indexes.map(index => {
						const stitchedAggregate = Object.assign({}, seedTransactions[index]);
						stitchedAggregate.transaction.transactions = [seedTransactions[index + 1], seedTransactions[index + 2]];
						return stitchedAggregate;
					}),
					startId: test.db.createObjectId(12),
					pageIdStep: 3 // difference between consecutive ids (1 + 2 dependent documents)
				};

				// region test cases

				describe('incoming', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([21, 12, 3]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([3]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryIncoming({ numDependentDocuments: 2 })
					}, basicTraits, dbTransactionTraits.incoming));
				});

				describe('outgoing', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([15, 12, 9]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([9]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryOutgoing({ numDependentDocuments: 2 })
					}, basicTraits, dbTransactionTraits.outgoing));
				});

				['all', 'unconfirmed', 'partial'].forEach(key => {
					describe(key, () => {
						addTests(Object.assign({
							getAccount2Transactions: basicTraits.curryFromIndexes([21, 15, 12, 9, 3]),
							getAccount2FilteredTransactions: basicTraits.curryFromIndexes([9, 3]),
							createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryAll({ numDependentDocuments: 2 })
						}, basicTraits, dbTransactionTraits[key]));
					});
				});

				// endregion
			});
		});

		const addParticipantTests = (aggregateType, setTransactionParticipants) => {
			// creates transactions for four accounts A, B, C (tested account), D
			// [0001] A -> B, [0002] A -> C, [0003] D -> D :: C, [0004] C -> B, [0005] C -> C, [0006] A -> B, [0007] A -> B :: D, C
			const createTransactionsWithParticipants = key => {
				const keys = [test.random.publicKey(), test.random.publicKey(), key, test.random.publicKey()];
				const addresses = keys.map(keyToAddress);

				const createTransactionTemplate = (signer, recipient, participants) => {
					const transaction = { type: aggregateType, signer: new Binary(signer), recipient: new Binary(recipient) };
					return { transaction, participants };
				};

				const transactionTemplates = [
					createTransactionTemplate(keys[0], addresses[1]),
					createTransactionTemplate(keys[0], addresses[2]),
					createTransactionTemplate(keys[3], addresses[3], [keys[2]]),
					createTransactionTemplate(keys[2], addresses[1]),
					createTransactionTemplate(keys[2], addresses[2]),
					createTransactionTemplate(keys[0], addresses[1]),
					createTransactionTemplate(keys[0], addresses[1], [keys[3], keys[2]])
				];

				let id = 1;
				const transactions = [];
				transactionTemplates.forEach(template => {
					// Arrange: set the participants if present (either transaction or meta needs to be modified)
					const { transaction } = template;
					const meta = createTransactionMetadata(transaction);
					if (template.participants)
						setTransactionParticipants(transaction, meta, template.participants);

					transactions.push({ _id: test.db.createObjectId(id++), meta, transaction });
				});

				return transactions;
			};

			const runBasicParticipantsTest = (traits, expectedTransactionIndexes) => {
				// Arrange:
				const key = test.random.publicKey();
				const seedTransactions = createTransactionsWithParticipants(key);
				return runDbTest(
					{ [getCollectionName(traits)]: seedTransactions },
					// Act: retrieve transactions for an account
					db => db[traits.dbFunctionName](key),
					transactions => {
						// Assert: expected transactions are available
						assertEqualDocuments(expectedTransactionIndexes.map(index => seedTransactions[index]), transactions);
					}
				);
			};

			// notice that incoming and outgoing apis do not include participant-only transactions
			it('is ignored by incoming', () => runBasicParticipantsTest(dbTransactionTraits.incoming, [4, 1]));
			it('is ignored by outgoing', () => runBasicParticipantsTest(dbTransactionTraits.outgoing, [4, 3]));

			const addSupportedParticipantsTests = traits => {
				const expectedTransactionIndexes = [6, 4, 3, 2, 1];
				it(`is included in ${traits.name}`, () => runBasicParticipantsTest(traits, expectedTransactionIndexes));
				it(`is included in ${traits.name} and pulls in all sub transactions`, () => {
					// Arrange:
					const key = test.random.publicKey();
					const seedTransactions = createTransactionsWithParticipants(key);

					// - for each seed transaction append two dependent documents
					const numAggregates = seedTransactions.length;
					for (let i = 0; i < numAggregates; ++i) {
						const aggregateId = seedTransactions[i]._id;
						const startId = numAggregates + (2 * i) + 1; // 1-based ids
						seedTransactions.push({ _id: test.db.createObjectId(startId), meta: { aggregateId }, transaction: {} });
						seedTransactions.push({ _id: test.db.createObjectId(startId + 1), meta: { aggregateId }, transaction: {} });
					}

					return runDbTest(
						{ [getCollectionName(traits)]: seedTransactions },
						// Act: retrieve transactions for an account
						db => db[traits.dbFunctionName](key),
						transactions => {
							// Assert: expected transactions are available and have stitched sub documents
							assertEqualDocuments(expectedTransactionIndexes.map(index => {
								const dependentStartIndex = numAggregates + (2 * index);
								const stitchedAggregate = seedTransactions[index];
								stitchedAggregate.transaction.transactions = [
									seedTransactions[dependentStartIndex],
									seedTransactions[dependentStartIndex + 1]
								];
								return stitchedAggregate;
							}), transactions);
						}
					);
				});
			};

			['all', 'unconfirmed', 'partial'].forEach(key => {
				addSupportedParticipantsTests(Object.assign({ name: key }, dbTransactionTraits[key]));
			});
		};

		const aggregateDescriptors = [
			{ name: 'aggregate complete', type: EntityType.aggregateComplete },
			{ name: 'aggregate bonded', type: EntityType.aggregateBonded }
		];

		aggregateDescriptors.forEach(aggregateDescriptor => {
			describe(aggregateDescriptor.name, () => {
				describe('cosignature only', () => {
					// Arrange: add participants as cosigners
					addParticipantTests(aggregateDescriptor.type, (transaction, meta, participants) => {
						transaction.cosignatures = participants.map(cosigner => ({ signer: new Binary(cosigner) }));
					});
				});

				describe('participant only', () => {
					// Arrange: add participants as affected addresses
					addParticipantTests(aggregateDescriptor.type, (transaction, meta, participants) => {
						meta.addresses = meta.addresses.concat(participants.map(participant => keyToAddress(participant)));
					});
				});
			});
		});
	});

	describe('account get', () => {
		const publicKey = test.random.publicKey();
		const decodedAddress = keyToAddress(publicKey);
		const publicKeyUnknown = test.random.publicKey();
		const decodedAddressUnknown = keyToAddress(publicKeyUnknown);

		const transformDbAccount = (dbAccountDocument, numImportances) => {
			// the db call should replace importances with the most recent importance and importance height,
			// so update the expected object to match
			const accountWithMetadata = Object.assign({}, dbAccountDocument);
			const { account } = accountWithMetadata;
			account.importance = Long.fromNumber(numImportances);
			account.importanceHeight = Long.fromNumber(numImportances * numImportances);
			delete account.importances;
			return accountWithMetadata;
		};

		const runSingleKnownAccountTest = (description, accountId, options) => {
			it(description, () => {
				// Arrange:
				// note: createAccounts uses options.numImportances to seed the importances for the accounts
				//       with values i for the importance and i * i for the importance height (0 < i <= numImportances)
				//       the last entry thus is (numImportances, numImportances * numImportances)
				const seedAccounts = test.db.createAccounts(publicKey, options);

				// Assert:
				return runDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([accountId]),
					accounts => {
						// Assert: compare against the transformed account instead of the db account
						const account = transformDbAccount(seedAccounts[0], options.numImportances);
						expect(accounts).to.deep.equal([account]);
					}
				);
			});
		};

		const runUnknownAccountTest = (accountId, options) => {
			it('returns empty array for unknown ids', () =>
				// Assert:
				runDbTest(
					{ accounts: test.db.createAccounts(publicKey, options) },
					db => db.accountsByIds([accountId]),
					accounts => expect(accounts).to.deep.equal([])
				));
		};

		const addAccountsByIdsTestsForSingleAccountLookup = (knownAccountId, unknownAccountId) => {
			runSingleKnownAccountTest(
				'can retrieve account with neither public key nor mosaics',
				knownAccountId,
				{ savePublicKey: false, numMosaics: 0 }
			);
			runSingleKnownAccountTest(
				'can retrieve account with public key but no mosaics',
				knownAccountId,
				{ savePublicKey: true, numMosaics: 0 }
			);
			runSingleKnownAccountTest(
				'can retrieve account without public key but with mosaics',
				knownAccountId,
				{ savePublicKey: false, numMosaics: 3 }
			);
			runSingleKnownAccountTest(
				'can retrieve account with public key and with mosaics',
				knownAccountId,
				{ savePublicKey: true, numMosaics: 3 }
			);
			runSingleKnownAccountTest(
				'can retrieve account without importances',
				knownAccountId,
				{ savePublicKey: true, numMosaics: 3, numImportances: 0 }
			);
			runSingleKnownAccountTest(
				'can retrieve account with single importance',
				knownAccountId,
				{ savePublicKey: true, numMosaics: 3, numImportances: 1 }
			);
			runUnknownAccountTest(
				unknownAccountId,
				{ savePublicKey: false, numMosaics: 3, expectedAddress: decodedAddressUnknown }
			);
		};

		describe('account from decoded address', () => {
			addAccountsByIdsTestsForSingleAccountLookup({ address: decodedAddress }, { address: decodedAddressUnknown });
		});

		describe('account from public key', () => {
			// note: even if public key is not known in the accounts collection, the call to accountsByIds()
			//       will succeed since the public key is converted to a decoded address.
			addAccountsByIdsTestsForSingleAccountLookup({ publicKey }, { publicKey: publicKeyUnknown });
		});

		describe('multiple accounts', () => {
			const runMultipleAccountsByIdsTests = runTest => {
				// Arrange: create 5 random accounts and extract their public keys
				const seedAccounts = test.db.createAccounts(test.random.publicKey(), {
					numAccounts: 4,
					savePublicKey: true,
					saveRandomPublicKey: true,
					numMosaics: 3,
					numImportances: 1
				});
				const publicKeys = seedAccounts.map(seedAccount => seedAccount.account.publicKey.buffer);

				// Assert:
				return runTest(seedAccounts, publicKeys);
			};

			it('returns multiple matching accounts', () =>
				// Arrange:
				runMultipleAccountsByIdsTests((seedAccounts, publicKeys) => runDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([
						{ publicKey: publicKeys[1] },
						{ address: keyToAddress(publicKeys[3]) },
						{ publicKey: publicKeys[4] }
					]),
					accounts => expect(accounts).to.deep.equal([1, 3, 4].map(index => transformDbAccount(seedAccounts[index], 1)))
				)));

			it('returns only known matching accounts', () =>
				// Arrange:
				runMultipleAccountsByIdsTests((seedAccounts, publicKeys) => runDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([
						{ publicKey: publicKeys[1] },
						{ publicKey: test.random.publicKey() },
						{ address: test.random.address() },
						{ address: keyToAddress(publicKeys[3]) }
					]),
					accounts => expect(accounts).to.deep.equal([1, 3].map(index => transformDbAccount(seedAccounts[index], 1)))
				)));
		});
	});

	// region failed transactions

	describe('failed transactions by hashes', () => {
		const runTransactionsByHashesFailedTest = (numSeeds, runTest) => {
			// Arrange:
			const hashes = Array.from(Array(numSeeds), () => test.random.hash());
			const failedTransactionResults = [];
			for (let i = 0; i < numSeeds; ++i)
				failedTransactionResults.push({ hash: new Binary(hashes[i]), validationResult: i });

			// Assert:
			return runTest(failedTransactionResults, hashes);
		};

		it('returns empty array for unknown hashes', () =>
			// Arrange:
			runTransactionsByHashesFailedTest(3, seedResults => runDbTest(
				{ transactionStatuses: seedResults },
				db => db.transactionsByHashesFailed([test.random.hash(), test.random.hash()]),
				results => { expect(results).to.deep.equal([]); }
			)));

		it('returns single matching failed transaction', () =>
			// Arrange:
			runTransactionsByHashesFailedTest(3, (seedResults, hashes) => runDbTest(
				{ transactionStatuses: seedResults },
				db => db.transactionsByHashesFailed([hashes[1]]),
				results => { expect(results).to.deep.equal([seedResults[1]]); }
			)));

		it('returns multiple matching failed transactions', () =>
			// Arrange:
			runTransactionsByHashesFailedTest(5, (seedResults, hashes) => runDbTest(
				{ transactionStatuses: seedResults },
				db => db.transactionsByHashesFailed([1, 3, 4].map(index => hashes[index])),
				results => { expect(results).to.deep.equal([1, 3, 4].map(index => seedResults[index])); }
			)));

		it('returns only known matching failed transactions', () =>
			// Arrange:
			runTransactionsByHashesFailedTest(3, (seedResults, hashes) => runDbTest(
				{ transactionStatuses: seedResults },
				db => db.transactionsByHashesFailed([hashes[0], test.random.hash(), hashes[2], test.random.hash()]),
				results => { expect(results).to.deep.equal([0, 2].map(index => seedResults[index])); }
			)));
	});

	// endregion
});
