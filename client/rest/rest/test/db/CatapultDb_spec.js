import { expect } from 'chai';
import catapult from 'catapult-sdk';
import MongoDb from 'mongodb';
import CatapultDb from '../../src/db/CatapultDb';
import test from './utils/dbTestUtils';
import testDbOptions from './utils/testDbOptions';

const address = catapult.model.address;

const Long = MongoDb.Long;
const Binary = MongoDb.Binary;
const Mijin_Test_Network = testDbOptions.networkId;
const Default_Height = 34567;

describe('catapult db', () => {
	function deleteIds(dbEntities) {
		for (const collectionName of test.collection.names) {
			const seed = test.collection.findInEntities(dbEntities, collectionName);
			test.db.sanitizeDbEntities(collectionName, seed);
		}
	}

	function assertDbCall(dbEntities, callAndAssert) {
		// Arrange:
		const db = new CatapultDb({ networkId: Mijin_Test_Network });

		// Act + Assert:
		return db.connect(testDbOptions.url, 'test')
			.then(() => test.db.populateDatabase(db, dbEntities))
			.then(() => deleteIds(dbEntities))
			.then(() => callAndAssert(db))
			.then(() => db.close());
	}

	function runDbTest(dbEntities, issueDbCommand, assertDbCommandResult) {
		// Arrange:
		const db = new CatapultDb({ networkId: Mijin_Test_Network });

		// Act + Assert:
		return db.connect(testDbOptions.url, 'test')
			.then(() => test.db.populateDatabase(db, dbEntities))
			.then(() => deleteIds(dbEntities))
			.then(() => issueDbCommand(db))
			.then(assertDbCommandResult)
			.then(() => db.close());
	}

	function assertEqualDocuments(expectedDocuments, actualDocuments) {
		function getAttributes(documents) {
			const documentToIdString = document => (document && document.meta ? document.meta.id.toString() : undefined);
			const subTxIds = documents.map(document => (document.transactions || []).map(documentToIdString));
			return {
				numDocuments: documents.length,
				ids: documents.map(documentToIdString),
				numSubTxes: subTxIds.reduce((sum, ids) => sum + ids.length, 0),
				subTxIds
			};
		}

		const expectedAttributes = getAttributes(expectedDocuments);
		const actualAttributes = getAttributes(actualDocuments);

		// Assert:
		expect(actualAttributes.numDocuments, 'wrong number of documents').to.equal(expectedAttributes.numDocuments);
		expect(actualAttributes.ids, 'wrong ids').to.deep.equal(expectedAttributes.ids);
		expect(actualAttributes.numSubTxes, 'wrong number of sub documents').to.equal(expectedAttributes.numSubTxes);
		expect(actualAttributes.subTxIds, 'wrong sub document ids').to.deep.equal(expectedAttributes.subTxIds);
		expect(actualDocuments).to.deep.equal(expectedDocuments);
	}

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
			const dbEntities = {
				block: test.db.createDbBlock(),
				transactions: test.db.createDbTransactions(Rounds, test.random.publicKey(), test.random.address())
			};

			const callAndAssert = db => db.storageInfo()
				.then(storageInfo => expect(storageInfo).to.deep.equal({ numBlocks: 1, numTransactions: Rounds * 8, numAccounts: 0 }));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});
	});

	describe('chain info', () => {
		it('can retrieve chain info', () => {
			// Arrange:
			const dbEntities = { chainInfo: test.db.createChainInfo(1357, 2468, 3579) };
			const callAndAssert = db => db.chainInfo().then(chainInfo => expect(chainInfo).to.deep.equal(dbEntities.chainInfo));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});
	});

	describe('block at height', () => {
		it('undefined is returned for block at unknown height', () => {
			// Arrange:
			const dbEntities = { block: test.db.createDbBlock(Default_Height) };
			const callAndAssert = db => db.blockAtHeight(Long.fromNumber(Default_Height + 1))
				.then(block => expect(block).to.equal(undefined));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		// use blockAtHeight tests as a proxy for testing support of different numeric types (Number, uint64, Long)

		function assertCanRetrieveSimpleBlock(height) {
			// Arrange:
			const dbEntities = { block: test.db.createDbBlock(Default_Height) };
			const callAndAssert = db => db.blockAtHeight(height)
				.then(block => expect(block).to.deep.equal(dbEntities.block));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		}

		it('can retrieve block without transactions at height (Number)', () => assertCanRetrieveSimpleBlock(Default_Height));
		it('can retrieve block without transactions at height (uint64)', () => assertCanRetrieveSimpleBlock([Default_Height, 0]));
		it('can retrieve block without transactions at height (Long)', () => assertCanRetrieveSimpleBlock(Long.fromNumber(Default_Height)));

		it('can retrieve block with transactions at height', () => {
			// Arrange:
			const blockTransactions = test.db.createDbTransactions(2, test.random.publicKey(), test.random.address());
			const dbEntities = { block: test.db.createDbBlock(Default_Height), transactions: blockTransactions };
			const callAndAssert = db => db.blockAtHeight(Long.fromNumber(Default_Height))
				.then(block => {
					expect(block).to.deep.equal(dbEntities.block);
				});

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});
	});

	describe('blocks from height', () => {
		function createBlocks(height, numBlocks) {
			const blocks = [];
			// the blocks are created in descending height order
			// in order to make later comparisons easier using .slice()
			for (let i = 0; i < numBlocks; ++i)
				blocks.push(test.db.createDbBlock(height + numBlocks - 1 - i));

			return blocks;
		}

		function createDbEntities(numBlocks) {
			return {
				chainInfo: test.db.createChainInfo(Default_Height + numBlocks - 1, 0, 0),
				blocks: createBlocks(Default_Height, numBlocks)
			};
		}

		it('returns empty array for unknown height', () => {
			// Arrange:
			const dbEntities = createDbEntities(1);

			// Act:
			const callAndAssert = db => db.blocksFrom(Long.fromNumber(Default_Height + 1), 10)
				.then(blocks => expect(blocks).to.deep.equal([]));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		function assertBlocks(actualBlocks, dbEntities, startHeight, numBlocks) {
			// Assert: actual blocks should contain `numBlocks` blocks from `startHeight`.
			// dbEntities.blocks are sorted in descending order, so:
			// 1. inside the expected list find the index of element with height `startHeight`,
			// 2. go back numBlocks elements to find element with largest height.
			const endElement = dbEntities.blocks.findIndex(entity => entity.block.height.toNumber() === startHeight) + 1;
			const startElement = endElement - numBlocks;
			expect(actualBlocks.length).to.equal(numBlocks);
			expect(actualBlocks).to.deep.equal(dbEntities.blocks.slice(startElement, endElement));
		}

		it('returns at most available blocks', () => {
			// Arrange:
			const dbEntities = createDbEntities(5);

			// Act:
			const callAndAssert = db => db.blocksFrom(Default_Height + 2, 10)
				.then(blocks => assertBlocks(blocks, dbEntities, Default_Height + 2, 3));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		it('respects requested number of blocks', () => {
			// Arrange:
			const dbEntities = createDbEntities(10);

			const callAndAssert = db => db.blocksFrom(Default_Height + 4, 3)
				.then(blocks => assertBlocks(blocks, dbEntities, Default_Height + 4, 3));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		it('returns empty array when requesting 0 blocks', () => {
			// Arrange:
			const dbEntities = createDbEntities(10);

			const callAndAssert = db => db.blocksFrom(Default_Height + 4, 0)
				.then(blocks => expect(blocks).to.deep.equal([]));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		it('returns top blocks when requesting from "0" height', () => {
			// Arrange:
			const dbEntities = createDbEntities(34);

			const callAndAssert = db => db.blocksFrom(0, 10)
				.then(blocks => assertBlocks(blocks, dbEntities, Default_Height + 24, 10));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		it('returns top blocks when requesting from "0" height even if more blocks are in the database', () => {
			// Arrange: set height to 4
			const dbEntities = {
				chainInfo: test.db.createChainInfo(4, 0, 0),
				blocks: createBlocks(1, 10)
			};

			const callAndAssert = db => db.blocksFrom(0, 10)
				.then(blocks => assertBlocks(blocks, dbEntities, 1, 4));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		it('returns last block when requesting from "0" height with alignment 1', () => {
			// Arrange: set height to 134
			const dbEntities = {
				chainInfo: test.db.createChainInfo(134, 0, 0),
				blocks: createBlocks(120, 15)
			};

			const callAndAssert = db => db.blocksFrom(0, 1)
				.then(blocks => assertBlocks(blocks, dbEntities, 134, 1));

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});

		it('returns blocks sorted in descending order', () => {
			// Arrange: try to insert in random order
			const dbEntities = {
				chainInfo: test.db.createChainInfo(Default_Height + 2, 0, 0),
				blocks: [
					test.db.createDbBlock(Default_Height + 2),
					test.db.createDbBlock(Default_Height),
					test.db.createDbBlock(Default_Height + 1)
				]
			};

			const callAndAssert = db => db.blocksFrom(Default_Height, 5)
				.then(blocks => {
					// Assert: blocks are returned in proper order - descending by height
					expect(blocks.length).to.equal(3);
					for (let i = 0; 3 > i; ++i)
						expect(blocks[i].block.height).to.deep.equal(Long.fromNumber(Default_Height + 2 - i));
				});

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		});
	});

	describe('transactions at height', () => {
		function createSeedTransactions(numTransactionsPerHeight, heights, options) {
			// notice that generated transactions only contain what is used by transactionsAtHeight for filtering (meta.height)
			let id = 1;
			const transactions = [];
			for (let i = 0; i < numTransactionsPerHeight; ++i) {
				for (const height of heights) {
					const aggregateId = test.db.createObjectId(id++);
					transactions.push({ _id: aggregateId, meta: { height }, transaction: { type: catapult.model.EntityType.aggregate } });

					const numDependentDocuments = (options || {}).numDependentDocuments || 0;
					for (let j = 0; j < numDependentDocuments; ++j)
						transactions.push({ _id: test.db.createObjectId(id++), meta: { height, aggregateId }, transaction: {} });
				}
			}

			return transactions;
		}

		function addTests(traits) {
			it('returns empty array for unknown height', () => {
				// Arrange: at heights 17, 25 and 36 - for each height create 3 transactions
				const seedTransactions = traits.createSeedTransactions(3);
				const dbEntities = { transactions: seedTransactions };

				// Act:
				return runDbTest(
					dbEntities,
					db => db.transactionsAtHeight(30),
					transactions => {
						// Assert: no transactions are available at height 30
						assertEqualDocuments([], transactions);
					});
			});

			it('can retrieve all transactions at height', () => {
				// Arrange: at heights 17, 25 and 36 - for each height create 3 transactions
				const seedTransactions = traits.createSeedTransactions(3);
				const dbEntities = { transactions: seedTransactions };

				// Act:
				return runDbTest(
					dbEntities,
					db => db.transactionsAtHeight(25),
					transactions => {
						// Assert: all three transactions at height 25 are returned
						assertEqualDocuments(traits.getHeight25Transactions(seedTransactions, [0, 1, 2]), transactions);
					});
			});

			it('query respects supplied id', () => {
				// Arrange: at heights 17, 25 and 36 - for each height create 3 transactions
				const seedTransactions = traits.createSeedTransactions(3);
				const dbEntities = { transactions: seedTransactions };

				return runDbTest(
					dbEntities,
					// Act: query passing the id of the second transaction
					db => db.transactionsAtHeight(25, traits.getHeight25Transactions(seedTransactions, [1])[0].meta.id.toString()),
					transactions => {
						// Assert: only the transactions after the passed id should be returned
						assertEqualDocuments(traits.getHeight25Transactions(seedTransactions, [2]), transactions);
					});
			});

			function assertPageSize(numTransactionsPerHeight, pageSize, numExpectedTransactions) {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions(numTransactionsPerHeight);
				const dbEntities = { transactions: seedTransactions };

				return runDbTest(
					dbEntities,
					// Act: query with a custom page size
					db => db.transactionsAtHeight(25, undefined, pageSize),
					transactions => {
						// Assert: at most a single page was returned starting from the first transaction
						const expectedIndexes = [];
						for (let i = 0; i < numExpectedTransactions; ++i)
							expectedIndexes.push(i);

						assertEqualDocuments(traits.getHeight25Transactions(seedTransactions, expectedIndexes), transactions);
					});
			}

			// minimum and maximum values are set in CatapultDb ctor
			it('query respects page size', () => assertPageSize(14, 12, 12));
			it('query ensures minimum page size', () => assertPageSize(14, 3, 10));
			it('query ensures maximum page size', () => assertPageSize(150, 125, 100));
		}

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
		function createSeedTransactions() {
			// notice that transactionById is only dependent on the id, so document content is immaterial
			return [
				{ _id: test.db.createObjectId(1), meta: {}, payload: 'foo' },
				{ _id: test.db.createObjectId(2), meta: {}, payload: 'bar' },
				{ _id: test.db.createObjectId(3), meta: {}, payload: 'baz' }
			];
		}

		it('can retrieve each transaction by id', () => {
			// Arrange:
			const transactions = createSeedTransactions();

			// Act + Assert:
			return runDbTest(
				{ transactions },
				db => {
					const promises = [];
					for (let i = 1; transactions.length >= i; ++i)
						promises.push(db.transactionById(test.db.createObjectId(i)));

					return Promise.all(promises);
				},
				resultTransactions => expect(resultTransactions).to.deep.equal(transactions));
		});

		it('can retrieve transaction using known id', () => {
			// Arrange:
			const transactions = createSeedTransactions();
			const existentId = '000000000000000000000002';

			// Act + Assert:
			return runDbTest(
				{ transactions },
				db => db.transactionById(existentId),
				transaction => expect(transaction).to.deep.equal(transactions[1]));
		});

		it('cannot retrieve transaction using unknown id', () => {
			// Arrange:
			const transactions = createSeedTransactions();
			const nonExistentId = '000000000000000000000004';

			// Act + Assert:
			return runDbTest(
				{ transactions },
				db => db.transactionById(nonExistentId),
				transaction => expect(transaction).to.equal(undefined));
		});
	});

	describe('names by ids', () => {
		function createDbMarkedTransaction(id, parentId, markerId) {
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
		}

		function createDbMarkedTransactions(numTransactions, numRepetitions) {
			const transactions = [];
			let id = 0;
			// create multiple (numRepetitions) transactions with same markerId, but different _id field.
			for (let i = 0; i < numRepetitions; ++i) {
				for (let j = 0; j < numTransactions; ++j)
					transactions.push(createDbMarkedTransaction(test.db.createObjectId(++id), 15000 + j, 20000 + j));
			}

			return transactions;
		}

		function createDbEntities() {
			return { transactions: createDbMarkedTransactions(12, 3) };
		}

		function assertTransactions(expectedTransactions, ids) {
			// Arrange: seed with transactions with markerIds: 20000 - 20011
			const dbEntities = createDbEntities();

			const callAndAssert = db => db.findNamesByIds(ids, 0x12345,
				{
					id: 'markerId',
					name: 'markerName',
					parentId: 'parentMarkerId'
				})
				.then(transactions => {
					expect(transactions.length).to.equal(expectedTransactions.length);
					expect(transactions).to.deep.equal(expectedTransactions);
				});

			// Assert:
			return assertDbCall(dbEntities, callAndAssert);
		}

		function createExpected(parentId, markerId) {
			return {
				markerId: Long.fromNumber(markerId),
				markerName: `marker-${markerId}`,
				parentMarkerId: Long.fromNumber(parentId)
			};
		}

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
		function getCollectionName(traits) {
			return [traits.collectionName || 'transactions'];
		}

		function keyToAddress(key) {
			return Buffer.from(address.publicKeyToAddress(key, Mijin_Test_Network));
		}

		const dbTransactionTraits = {
			incoming: { dbFunctionName: 'accountTransactionsIncoming' },
			outgoing: { dbFunctionName: 'accountTransactionsOutgoing' },
			all: { dbFunctionName: 'accountTransactionsAll' },
			unconfirmed: { dbFunctionName: 'accountTransactionsUnconfirmed', collectionName: 'unconfirmedTransactions' }
		};

		describe('can filter by', () => {
			// creates transactions for three accounts such that: A is sender, B is recipient, C is sender and recipient
			// [0001] A -> B, [0002] A -> C, [0003] C -> B, [0004] C -> C
			function createDirectionalTransactions(key1, key2, key3) {
				const keys = [key1, key2, key3];
				const addresses = keys.map(keyToAddress);

				function createTransactionTemplate(signer, recipient) {
					return { signer: new Binary(signer), recipient: new Binary(recipient) };
				}

				const transactionTemplates = [
					createTransactionTemplate(keys[0], addresses[1]),
					createTransactionTemplate(keys[0], addresses[2]),
					createTransactionTemplate(keys[2], addresses[1]),
					createTransactionTemplate(keys[2], addresses[2])
				];

				let id = 1;
				const transactions = [];
				for (const transaction of transactionTemplates)
					transactions.push({ _id: test.db.createObjectId(id++), meta: {}, transaction });

				return transactions;
			}

			function addTests(traits) {
				function addDirectionalTest(keySelector, expectedTransactionIndexes) {
					// Arrange:
					const keys = [test.random.publicKey(), test.random.publicKey(), test.random.publicKey()];
					const seedTransactions = createDirectionalTransactions(keys[0], keys[1], keys[2]);
					const dbEntities = { [getCollectionName(traits)]: seedTransactions };

					return runDbTest(
						dbEntities,
						// Act: retrieve transactions for an account
						db => db[traits.dbFunctionName](keySelector(keys)),
						transactions => {
							// Assert: expected transactions are available
							assertEqualDocuments(expectedTransactionIndexes.map(index => seedTransactions[index]), transactions);
						});
				}

				it('for account without transactions', () =>
					addDirectionalTest(() => test.random.publicKey(), [])); // random

				it('for account with outgoing only transactions', () =>
					addDirectionalTest(keys => keys[0], traits.directional.outgoing)); // A

				it('for account with incoming only transactions', () =>
					addDirectionalTest(keys => keys[1], traits.directional.incoming)); // B

				it('for account with incoming and outgoing transactions', () =>
					addDirectionalTest(keys => keys[2], traits.directional.incomingAndOutgoing)); // C
			}

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

			describe('all', () => {
				addTests(Object.assign({
					directional: { outgoing: [1, 0], incoming: [2, 0], incomingAndOutgoing: [3, 2, 1] }
				}, dbTransactionTraits.all));
			});

			describe('unconfirmed', () => {
				addTests(Object.assign({
					directional: { outgoing: [1, 0], incoming: [2, 0], incomingAndOutgoing: [3, 2, 1] }
				}, dbTransactionTraits.unconfirmed));
			});
		});

		describe('can page', () => {
			function addTransactions(transactions, id, signer, recipient, numDependentDocuments) {
				const aggregateId = test.db.createObjectId(id);
				const aggregateType = catapult.model.EntityType.aggregate;
				transactions.push({
					_id: aggregateId,
					meta: {},
					transaction: { type: aggregateType, signer: new Binary(signer), recipient: new Binary(recipient) }
				});

				for (let j = 0; j < numDependentDocuments; ++j)
					transactions.push({ _id: test.db.createObjectId(id + j + 1), meta: { aggregateId }, transaction: {} });
			}

			function createBilateralTransactions(keys, options) {
				const addresses = keys.map(keyToAddress);

				let id = 0;
				const transactions = [];
				for (const signer of keys) {
					for (const recipient of addresses) {
						const numDependentDocuments = (options || {}).numDependentDocuments || 0;
						addTransactions(transactions, id, signer, recipient, numDependentDocuments);
						id += 1 + numDependentDocuments;
					}
				}

				return transactions;
			}

			function createAlternatingTransactions(numMatchingTransactions, createMatchingSenderRecipientPair, options) {
				const transactions = [];
				const randomKey = test.random.publicKey();
				const randomAddress = keyToAddress(test.random.publicKey());

				let id = 0;
				for (let i = 0; i < 2 * numMatchingTransactions; ++i) {
					// create a matching transaction every even iteration
					const senderRecipientPair = 0 === id % 2 ? createMatchingSenderRecipientPair() : { signer: randomKey, recipient: randomAddress };
					const numDependentDocuments = (options || {}).numDependentDocuments || 0;
					addTransactions(transactions, id, senderRecipientPair.signer, senderRecipientPair.recipient, numDependentDocuments);
					id += 1 + numDependentDocuments;
				}

				return transactions;
			}

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
						options);
				}
			};

			function addTests(traits) {
				it('can retrieve all matching transactions', () => {
					// Arrange:
					const keys = [test.random.publicKey(), test.random.publicKey(), test.random.publicKey()];
					const seedTransactions = traits.createSeedTransactions(keys);
					const dbEntities = { [getCollectionName(traits)]: seedTransactions };

					return runDbTest(
						dbEntities,
						// Act: retrieve transactions from the second account
						db => db[traits.dbFunctionName](keys[1]),
						transactions => {
							// Assert: only the transactions matching the second account were returned
							assertEqualDocuments(traits.getAccount2Transactions(seedTransactions), transactions);
						});
				});

				it('query respects supplied id', () => {
					// Arrange:
					const keys = [test.random.publicKey(), test.random.publicKey(), test.random.publicKey()];
					const seedTransactions = traits.createSeedTransactions(keys);
					const dbEntities = { [getCollectionName(traits)]: seedTransactions };

					return runDbTest(
						dbEntities,
						// Act: query passing a custom id
						db => db[traits.dbFunctionName](keys[1], traits.startId),
						transactions => {
							// Assert: only the transactions after the passed id should be returned
							assertEqualDocuments(traits.getAccount2FilteredTransactions(seedTransactions, [2]), transactions);
						});
				});

				function assertPageSize(numMatchingTransactions, pageSize, numExpectedTransactions) {
					// Arrange: generate transactions that alternate matching the filter; Y, N, Y, N ...
					const key = test.random.publicKey();
					const seedTransactions = traits.createPagingSeedTransactions(numMatchingTransactions, key);
					const dbEntities = { [getCollectionName(traits)]: seedTransactions };

					return runDbTest(
						dbEntities,
						// Act: query with a custom page size
						db => db[traits.dbFunctionName](key, undefined, pageSize),
						transactions => {
							// Assert: at most a single page was returned (subtract 1 because oids are 0-based)
							const expectedIndexes = [];
							for (let i = 0; i < numExpectedTransactions; ++i)
								expectedIndexes.push(((numMatchingTransactions - i - 1) * 2) * traits.pageIdStep);

							assertEqualDocuments(traits.curryFromIndexes(expectedIndexes)(seedTransactions), transactions);
						});
				}

				// minimum and maximum values are set in CatapultDb ctor
				it('query respects page size', () => assertPageSize(14, 12, 12));
				it('query ensures minimum page size', () => assertPageSize(14, 3, 10));
				it('query ensures maximum page size', () => assertPageSize(150, 125, 100));
			}

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

				describe('all', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([7, 5, 4, 3, 1]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([3, 1]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryAll()
					}, basicTraits, dbTransactionTraits.all));
				});

				describe('unconfirmed', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([7, 5, 4, 3, 1]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([3, 1]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryAll()
					}, basicTraits, dbTransactionTraits.unconfirmed));
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

				describe('all', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([21, 15, 12, 9, 3]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([9, 3]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryAll({ numDependentDocuments: 2 })
					}, basicTraits, dbTransactionTraits.all));
				});

				describe('unconfirmed', () => {
					addTests(Object.assign({
						getAccount2Transactions: basicTraits.curryFromIndexes([21, 15, 12, 9, 3]),
						getAccount2FilteredTransactions: basicTraits.curryFromIndexes([9, 3]),
						createPagingSeedTransactions: createPagingSeedTransactionsFactory.curryAll({ numDependentDocuments: 2 })
					}, basicTraits, dbTransactionTraits.unconfirmed));
				});

				// endregion
			});
		});

		describe('cosignature only', () => {
			// creates transactions for four accounts A, B, C (tested account), D
			// [0001] A -> B, [0002] A -> C, [0003] D -> D :: C, [0004] C -> B, [0005] C -> C, [0006] A -> B, [0007] A -> B :: D, C
			function createTransactionsWithCosignatures(key) {
				const keys = [test.random.publicKey(), test.random.publicKey(), key, test.random.publicKey()];
				const addresses = keys.map(keyToAddress);

				function createTransactionTemplate(signer, recipient, cosigners) {
					const aggregateType = catapult.model.EntityType.aggregate;
					const template = { type: aggregateType, signer: new Binary(signer), recipient: new Binary(recipient) };
					if (cosigners)
						template.cosignatures = cosigners.map(cosigner => ({ signer: new Binary(cosigner) }));

					return template;
				}

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
				for (const transaction of transactionTemplates)
					transactions.push({ _id: test.db.createObjectId(id++), meta: {}, transaction });

				return transactions;
			}

			function runBasicCosignaturesTest(traits, expectedTransactionIndexes) {
				// Arrange:
				const key = test.random.publicKey();
				const seedTransactions = createTransactionsWithCosignatures(key);
				const dbEntities = { [getCollectionName(traits)]: seedTransactions };

				return runDbTest(
					dbEntities,
					// Act: retrieve transactions for an account
					db => db[traits.dbFunctionName](key),
					transactions => {
						// Assert: expected transactions are available
						assertEqualDocuments(expectedTransactionIndexes.map(index => seedTransactions[index]), transactions);
					});
			}

			// notice that incoming and outgoing apis do not include cosigned-only transactions
			it('is ignored by incoming', () => runBasicCosignaturesTest(dbTransactionTraits.incoming, [4, 1]));
			it('is ignored by outgoing', () => runBasicCosignaturesTest(dbTransactionTraits.outgoing, [4, 3]));

			function addSupportedCosignaturesTests(traits) {
				const expectedTransactionIndexes = [6, 4, 3, 2, 1];
				it(`is included in ${traits.name}`, () => runBasicCosignaturesTest(traits, expectedTransactionIndexes));
				it(`is included in ${traits.name} and pulls in all sub transactions`, () => {
					// Arrange:
					const key = test.random.publicKey();
					const seedTransactions = createTransactionsWithCosignatures(key);
					const dbEntities = { [getCollectionName(traits)]: seedTransactions };

					// - for each seed transaction append two dependent documents
					const numAggregates = seedTransactions.length;
					for (let i = 0; i < numAggregates; ++i) {
						const aggregateId = seedTransactions[i]._id;
						const startId = numAggregates + (2 * i) + 1; // 1-based ids
						seedTransactions.push({ _id: test.db.createObjectId(startId), meta: { aggregateId }, transaction: {} });
						seedTransactions.push({ _id: test.db.createObjectId(startId + 1), meta: { aggregateId }, transaction: {} });
					}

					return runDbTest(
						dbEntities,
						// Act: retrieve transactions for an account
						db => db[traits.dbFunctionName](key),
						transactions => {
							// Assert: expected transactions are available and have stitched sub documents
							assertEqualDocuments(expectedTransactionIndexes.map(index => {
								const dependentStartIndex = numAggregates + (2 * index);
								const stitchedAggregate = Object.assign({}, seedTransactions[index]);
								stitchedAggregate.transaction.transactions = [
									seedTransactions[dependentStartIndex],
									seedTransactions[dependentStartIndex + 1]
								];
								return stitchedAggregate;
							}), transactions);
						});
				});
			}

			addSupportedCosignaturesTests(Object.assign({ name: 'all' }, dbTransactionTraits.all));
			addSupportedCosignaturesTests(Object.assign({ name: 'unconfirmed' }, dbTransactionTraits.unconfirmed));
		});
	});

	describe('account get', () => {
		const publicKey = test.random.publicKey();
		const decodedAddress = address.publicKeyToAddress(publicKey, Mijin_Test_Network);
		const publicKeyUnknown = test.random.publicKey();
		const decodedAddressUnknown = address.publicKeyToAddress(publicKeyUnknown, Mijin_Test_Network);

		function retrieveKnownAccountTest(description, dbCall, options) {
			it(description, () => {
				// Arrange:
				// note: createAccounts uses options.numImportances to seed the importances for the accounts
				//       with values i for the importance and i * i for the importance height (0 < i <= numImportances)
				//       the last entry thus is (numImportances, numImportances * numImportances)
				const accounts = test.db.createAccounts(publicKey, options);
				const dbEntities = { accounts };

				// Assert:
				const callAndAssert = db => dbCall(db)
					.then(accountWithMetaData => {
						// the db call should replace importances with the most recent importance and importance height,
						// so update the expected object to match
						const value = options.numImportances;
						const account = accounts[0].account;
						account.importance = Long.fromNumber(value);
						account.importanceHeight = Long.fromNumber(value * value);
						delete account.importances;
						expect(accountWithMetaData).to.deep.equal(accounts[0]);
					});

				// Assert:
				return assertDbCall(dbEntities, callAndAssert);
			});
		}

		function retrieveUnknownAccountTest(dbCall, options) {
			it('undefined is returned for unknown account', () => {
				// Arrange:
				const accounts = test.db.createAccounts(publicKey, options);
				const dbEntities = { accounts };

				// Assert:
				const callAndAssert = db => dbCall(db)
					.then(account => expect(account).to.equal(undefined));

				// Assert:
				return assertDbCall(dbEntities, callAndAssert);
			});
		}

		function accountGetTests(knownAccountSupplier, unknownAccountSupplier) {
			retrieveKnownAccountTest(
					'can retrieve account with neither public key nor mosaics',
					knownAccountSupplier,
					{ savePublicKey: false, numMosaics: 0 });
			retrieveKnownAccountTest(
					'can retrieve account with public key but no mosaics',
					knownAccountSupplier,
					{ savePublicKey: true, numMosaics: 0 });
			retrieveKnownAccountTest(
					'can retrieve account without public key but with mosaics',
					knownAccountSupplier,
					{ savePublicKey: false, numMosaics: 3 });
			retrieveKnownAccountTest(
					'can retrieve account with public key and with mosaics',
					knownAccountSupplier,
					{ savePublicKey: true, numMosaics: 3 });
			retrieveKnownAccountTest(
					'can retrieve account without importances',
					knownAccountSupplier,
					{ savePublicKey: true, numMosaics: 3, numImportances: 0 });
			retrieveKnownAccountTest(
					'can retrieve account with single importance',
					knownAccountSupplier,
					{ savePublicKey: true, numMosaics: 3, numImportances: 1 });
			retrieveUnknownAccountTest(
					unknownAccountSupplier,
					{ savePublicKey: false, numMosaics: 3, expectedAddress: decodedAddressUnknown });
		}

		describe('account from decoded address', () => {
			accountGetTests(db => db.accountGet(decodedAddress), db => db.accountGet(decodedAddressUnknown));
		});

		describe('account from public key', () => {
			// note: even if public key is not known in the accounts collection, the call to accountGetFromPublicKey()
			//       will succeed since the public key is converted to a decoded address.
			accountGetTests(db => db.accountGetFromPublicKey(publicKey), db => db.accountGetFromPublicKey(publicKeyUnknown));
		});
	});
});
