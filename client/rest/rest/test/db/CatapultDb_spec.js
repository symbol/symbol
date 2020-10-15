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

const test = require('./utils/dbTestUtils');
const testDbOptions = require('./utils/testDbOptions');
const CatapultDb = require('../../src/db/CatapultDb');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { address, EntityType } = catapult.model;

const { Long, Binary } = MongoDb;
const Mijin_Test_Network = testDbOptions.networkId;
const Default_Height = 34567;

const DefaultPagingOptions = {
	pageSizeMin: 10,
	pageSizeMax: 100,
	pageSizeDefault: 20
};

const TransactionGroups = {
	confirmed: 'confirmed',
	unconfirmed: 'unconfirmed',
	partial: 'partial'
};

describe('catapult db', () => {
	const deleteIds = dbEntities => {
		test.collection.names.forEach(collectionName => {
			const seed = test.collection.findInEntities(dbEntities, collectionName);
			test.db.sanitizeDbEntities(collectionName, seed);
		});
	};

	const renameId = dbObject => {
		if (dbObject) {
			dbObject.id = dbObject._id;
			delete dbObject._id;
		}

		return dbObject;
	};

	const keyToAddress = key => Buffer.from(address.publicKeyToAddress(key, Mijin_Test_Network));

	const runDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) => {
		// Arrange:
		const db = new CatapultDb(Object.assign({ networkId: Mijin_Test_Network }, DefaultPagingOptions));

		// Act + Assert:
		return db.connect(testDbOptions.url, 'test', testDbOptions.connectionPoolSize)
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
			const documentToIdString = document => (document ? document.id.toString() : undefined);
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
					block: test.db.createDbBlock(Default_Height),
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
				{ chainStatistic: test.db.createChainStatistic(1357, 2468, 3579) },
				db => db.chainStatisticCurrent(),
				chainStatistic => expect(chainStatistic).to.deep.equal(test.db.createChainStatistic(1357, 2468, 3579).current)
			));
	});

	describe('latest finalized block', () => {
		const createFinalizationBlock = height => ({
			block: {
				height: Long.fromNumber(height),
				hash: new Binary(test.random.hash()),
				finalizationEpoch: 777,
				finalizationPoint: 888
			}
		});

		it('can retrieve latest finalized block info', () => {
			const finalizedBlocks = [
				createFinalizationBlock(5),
				createFinalizationBlock(8),
				createFinalizationBlock(2)
			];
			return runDbTest(
				{ finalizedBlocks },
				db => db.latestFinalizedBlock(),
				latestfinalizedBlock => expect(latestfinalizedBlock).to.deep.equal(finalizedBlocks[1])
			);
		});
	});

	const stripBlockFields = (block, fields) => {
		// block merkle trees are not exposed outside of the database class unless explicitly requested
		const modifiedBlock = Object.assign({}, block);
		fields.forEach(field => delete modifiedBlock.meta[field]);
		return modifiedBlock;
	};

	const stripExtraneousBlockInformation = block => stripBlockFields(block, ['transactionMerkleTree', 'statementMerkleTree']);

	describe('blocks', () => {
		const account1 = {
			publicKey: test.random.publicKey(),
			address: test.random.address()
		};
		const account2 = {
			publicKey: test.random.publicKey(),
			address: test.random.address()
		};

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: '_id',
			sortDirection: -1
		};

		const { createObjectId } = test.db;

		const createBlock = (objectId, height, signerPublicKey, beneficiaryAddress) => ({
			_id: createObjectId(objectId),
			meta: {},
			block: {
				height,
				signerPublicKey: signerPublicKey ? Buffer.from(signerPublicKey) : undefined,
				beneficiaryAddress: beneficiaryAddress ? Buffer.from(beneficiaryAddress) : undefined
			}
		});

		const runTestAndVerifyIds = (dbBlocks, dbQuery, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runDbTest(
				{ blocks: dbBlocks },
				dbQuery,
				blocksPage => {
					const returnedIds = blocksPage.data.map(t => t.id);
					expect(blocksPage.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbBlocks = [
				createBlock(10, 100, account1.publicKey, account2.address)
			];

			// Act + Assert:
			return runDbTest(
				{ blocks: dbBlocks },
				db => db.blocks(undefined, undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'meta', 'block'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('returns correct blocks when no filters are provided', () => {
			// Arrange:
			const dbBlocks = [
				createBlock(10, 1),
				createBlock(20, 1, account1.publicKey),
				createBlock(30, 1, account1.publicKey, account2.address)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbBlocks, db => db.blocks(undefined, undefined, paginationOptions), [10, 20, 30]);
		});

		it('all the provided filters are taken into account', () => {
			// Arrange:
			const dbBlocks = [
				createBlock(10, 1),
				createBlock(20, 1, account1.publicKey),
				createBlock(30, 1, account1.publicKey, account2.address)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbBlocks, db => db.blocks(account1.publicKey, account2.address, paginationOptions), [30]);
		});

		describe('respects offset', () => {
			// Arrange:
			const dbBlocks = () => [
				createBlock(10, 20),
				createBlock(20, 30),
				createBlock(30, 10)
			];
			const options = {
				pageSize: 10,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1,
				offset: createObjectId(20)
			};

			it('gt', () => {
				options.sortDirection = 1;

				// Act + Assert:
				return runTestAndVerifyIds(dbBlocks(), db => db.blocks(undefined, undefined, options), [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbBlocks(), db => db.blocks(undefined, undefined, options), [10]);
			});
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbBlocks = () => [
				createBlock(10, 20),
				createBlock(20, 30),
				createBlock(30, 10)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runDbTest(
					{ blocks: dbBlocks() },
					db => db.blocks(undefined, undefined, options),
					blocksPage => {
						expect(blocksPage.data[0].id).to.deep.equal(createObjectId(10));
						expect(blocksPage.data[1].id).to.deep.equal(createObjectId(20));
						expect(blocksPage.data[2].id).to.deep.equal(createObjectId(30));
					}
				);
			});

			it('direction descending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: -1
				};

				// Act + Assert:
				return runDbTest(
					{ blocks: dbBlocks() },
					db => db.blocks(undefined, undefined, options),
					blocksPage => {
						expect(blocksPage.data[0].id).to.deep.equal(createObjectId(30));
						expect(blocksPage.data[1].id).to.deep.equal(createObjectId(20));
						expect(blocksPage.data[2].id).to.deep.equal(createObjectId(10));
					}
				);
			});

			it('sort field', () => {
				const queryPagedDocumentsSpy = sinon.spy(CatapultDb.prototype, 'queryPagedDocuments');
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'height',
					sortDirection: 1
				};

				// Act + Assert:
				return runDbTest(
					{ blocks: dbBlocks() },
					db => db.blocks(undefined, undefined, options),
					() => {
						expect(queryPagedDocumentsSpy.calledOnce).to.equal(true);
						expect(Object.keys(queryPagedDocumentsSpy.firstCall.args[2])[0]).to.equal('block.height');
						queryPagedDocumentsSpy.restore();
					}
				);
			});
		});

		describe('correctly applies each filter:', () => {
			it('signerPublicKey', () => {
				// Arrange:
				const dbBlocks = [
					createBlock(10, 1, account1.publicKey),
					createBlock(20, 1, account2.publicKey)
				];

				// Act + Assert:
				return runTestAndVerifyIds(dbBlocks, db => db.blocks(account1.publicKey, undefined, paginationOptions), [10]);
			});

			it('beneficiaryAddress', () => {
				// Arrange:
				const dbBlocks = [
					createBlock(10, 1, account1.publicKey, account1.address),
					createBlock(20, 1, account1.publicKey, account2.address)
				];

				// Act + Assert:
				return runTestAndVerifyIds(dbBlocks, db => db.blocks(undefined, account1.address, paginationOptions), [10]);
			});
		});
	});

	describe('block at height', () => {
		const runBlockAtHeightDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) => {
			const db = new CatapultDb(Object.assign({ networkId: Mijin_Test_Network }, DefaultPagingOptions));
			return db.connect(testDbOptions.url, 'test', testDbOptions.connectionPoolSize)
				.then(() => test.db.populateDatabase(db, dbEntities))
				.then(() => issueDbCommand(db))
				.then(assertDbCommandResult)
				.then(() => db.close());
		};

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
			return runBlockAtHeightDbTest(
				{ blocks: seedBlock },
				db => db.blockAtHeight(height),
				block => expect(block).to.deep.equal(stripExtraneousBlockInformation(renameId(seedBlock)))
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
			return runBlockAtHeightDbTest(
				{ blocks: seedBlock, transactions: blockTransactions },
				db => db.blockAtHeight(Long.fromNumber(Default_Height)),
				block => expect(block).to.deep.equal(stripExtraneousBlockInformation(renameId(seedBlock)))
			);
		});
	});

	describe('block at height with statement merkle tree', () => {
		it('undefined is returned for block at unknown height', () =>
			// Assert:
			runDbTest(
				{ block: test.db.createDbBlock(Default_Height) },
				db => db.blockWithMerkleTreeAtHeight(Long.fromNumber(Default_Height + 1), 'statementMerkleTree'),
				block => expect(block).to.equal(undefined)
			));

		// use blockAtHeight tests as a proxy for testing support of different numeric types (Number, uint64, Long)

		const assertCanRetrieveSimpleBlock = height => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);

			// Assert:
			return runDbTest(
				{ block: seedBlock },
				db => db.blockWithMerkleTreeAtHeight(height, 'statementMerkleTree'),
				block => expect(block).to.deep.equal(stripBlockFields(seedBlock, ['transactionMerkleTree']))
			);
		};

		it('can retrieve block with statement merkle tree at height (Number)', () => assertCanRetrieveSimpleBlock(Default_Height));
		it('can retrieve block with statement merkle tree at height (uint64)', () => assertCanRetrieveSimpleBlock([Default_Height, 0]));
		it('can retrieve block with statement merkle tree at height (Long)', () =>
			assertCanRetrieveSimpleBlock(Long.fromNumber(Default_Height)));

		it('can retrieve block with statement merkle tree at height', () => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);
			const blockTransactions = test.db.createDbTransactions(2, test.random.publicKey(), test.random.address());

			// Assert:
			return runDbTest(
				{ block: seedBlock, transactions: blockTransactions },
				db => db.blockWithMerkleTreeAtHeight(Long.fromNumber(Default_Height), 'statementMerkleTree'),
				block => expect(block).to.deep.equal(stripBlockFields(seedBlock, ['transactionMerkleTree']))
			);
		});
	});

	describe('block at height with transaction merkle tree', () => {
		it('undefined is returned for block at unknown height', () =>
			// Assert:
			runDbTest(
				{ block: test.db.createDbBlock(Default_Height) },
				db => db.blockWithMerkleTreeAtHeight(Long.fromNumber(Default_Height + 1), 'transactionMerkleTree'),
				block => expect(block).to.equal(undefined)
			));

		// use blockAtHeight tests as a proxy for testing support of different numeric types (Number, uint64, Long)

		const assertCanRetrieveSimpleBlock = height => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);

			// Assert:
			return runDbTest(
				{ block: seedBlock },
				db => db.blockWithMerkleTreeAtHeight(height, 'transactionMerkleTree'),
				block => expect(block).to.deep.equal(stripBlockFields(seedBlock, ['statementMerkleTree']))
			);
		};

		it('can retrieve block with transaction merkle tree at height (Number)', () => assertCanRetrieveSimpleBlock(Default_Height));
		it('can retrieve block with transaction merkle tree at height (uint64)', () => assertCanRetrieveSimpleBlock([Default_Height, 0]));
		it('can retrieve block with transaction merkle tree at height (Long)', () =>
			assertCanRetrieveSimpleBlock(Long.fromNumber(Default_Height)));

		it('can retrieve block with transaction merkle tree at height', () => {
			// Arrange:
			const seedBlock = test.db.createDbBlock(Default_Height);
			const blockTransactions = test.db.createDbTransactions(2, test.random.publicKey(), test.random.address());

			// Assert:
			return runDbTest(
				{ block: seedBlock, transactions: blockTransactions },
				db => db.blockWithMerkleTreeAtHeight(Long.fromNumber(Default_Height), 'transactionMerkleTree'),
				block => expect(block).to.deep.equal(stripBlockFields(seedBlock, ['statementMerkleTree']))
			);
		});
	});

	describe('latest blocks fee multiplier', () => {
		it('returns empty array when requesting 0 blocks', () => {
			// Arrange:
			const blockDbEntities = [
				{ block: { height: 10, feeMultiplier: 10 } },
				{ block: { height: 11, feeMultiplier: 11 } }
			];

			// Act + Assert:
			return runDbTest(
				{ blocks: blockDbEntities },
				db => db.latestBlocksFeeMultiplier(0),
				feeMultipliers => {
					expect(feeMultipliers).to.deep.equal([]);
				}
			);
		});

		it('returns latest available blocks based on height', () => {
			// Arrange:
			const blockDbEntities = [
				{ block: { height: 12, feeMultiplier: 12 } },
				{ block: { height: 15, feeMultiplier: 15 } },
				{ block: { height: 10, feeMultiplier: 10 } },
				{ block: { height: 14, feeMultiplier: 14 } },
				{ block: { height: 11, feeMultiplier: 11 } },
				{ block: { height: 13, feeMultiplier: 13 } }
			];

			// Act + Assert:
			return runDbTest(
				{ blocks: blockDbEntities },
				db => db.latestBlocksFeeMultiplier(5),
				feeMultipliers => {
					expect(feeMultipliers).to.deep.equal([15, 14, 13, 12, 11]);
				}
			);
		});

		it('respects requested number of blocks', () => {
			// Arrange:
			const blockDbEntities = [
				{ block: { height: 10, feeMultiplier: 1 } },
				{ block: { height: 11, feeMultiplier: 1 } },
				{ block: { height: 12, feeMultiplier: 1 } },
				{ block: { height: 13, feeMultiplier: 1 } },
				{ block: { height: 14, feeMultiplier: 1 } }
			];

			// Act + Assert:
			return runDbTest(
				{ blocks: blockDbEntities },
				db => db.latestBlocksFeeMultiplier(3),
				feeMultipliers => {
					expect(feeMultipliers.length).to.equal(3);
				}
			);
		});

		it('returns only fee multiplier values', () => {
			// Arrange:
			const blockDbEntities = [
				{
					meta: { hash: 'h1' },
					block: {
						network: 144,
						type: 33091,
						height: 11,
						feeMultiplier: 11.1
					}
				},
				{
					meta: { hash: 'h2' },
					block: {
						network: 144,
						type: 33091,
						height: 10,
						feeMultiplier: 10.1
					}
				}
			];

			// Act + Assert:
			return runDbTest(
				{ blocks: blockDbEntities },
				db => db.latestBlocksFeeMultiplier(5),
				feeMultipliers => {
					expect(feeMultipliers).to.deep.equal([11.1, 10.1]);
				}
			);
		});
	});

	const createTransactionHash = id => catapult.utils.convert.hexToUint8(`${'00'.repeat(16)}${id.toString(16)}`.slice(-32));

	const createSeedTransactions = (numTransactionsPerHeight, heights, options) => {
		// notice that generated transactions only contain what was used by transactionsAtHeight for filtering (meta.height)
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
			// transactionsAtHeight only worked for both aggregates, so pick each type alteratively
			const type = 0 === i % 2 ? EntityType.aggregateComplete : EntityType.aggregateBonded;
			heights.forEach(height => { addTransactionAtHeight(height, type); });
		}

		return transactions;
	};

	describe('transaction by id', () => {
		const runTransactionsDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) => {
			// Arrange:
			const db = new CatapultDb(Object.assign({ networkId: Mijin_Test_Network }, DefaultPagingOptions));

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test', testDbOptions.connectionPoolSize)
				.then(() => test.db.populateDatabase(db, dbEntities))
				.then(() => issueDbCommand(db))
				.then(assertDbCommandResult)
				.then(() => db.close());
		};

		const addTestsWithId = (traits, idTraits) => {
			it('can retrieve each transaction by id', () => {
				// Arrange:
				const seedTransactions = traits.createSeedTransactions();
				const allIds = traits.allIds.map(idTraits.convertToId);

				// Act + Assert:
				return runTransactionsDbTest(
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
				return runTransactionsDbTest(
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
				return runTransactionsDbTest(
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
				return runTransactionsDbTest(
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
					transactionsByIds: (db, ids) => db.transactionsByIds(TransactionGroups.confirmed, ids)
				}));

			describe('by transaction hash', () =>
				addTestsWithId(traits, {
					convertToId: createTransactionHash,
					collectionName: 'transactions',
					transactionsByIds: (db, ids) => db.transactionsByHashes(TransactionGroups.confirmed, ids)
				}));

			describe('by transaction hash (unconfirmed)', () =>
				addTestsWithId(traits, {
					convertToId: createTransactionHash,
					collectionName: 'unconfirmedTransactions',
					transactionsByIds: (db, ids) => db.transactionsByHashes(TransactionGroups.unconfirmed, ids)
				}));

			describe('by transaction hash (partial)', () =>
				addTestsWithId(traits, {
					convertToId: createTransactionHash,
					collectionName: 'partialTransactions',
					transactionsByIds: (db, ids) => db.transactionsByHashes(TransactionGroups.partial, ids)
				}));
		};

		describe('for transactions', () => {
			addTests({
				createSeedTransactions: () => createSeedTransactions(3, [21, 34]),
				expected: (transactions, ids) => ids.map(id => transactions[id - 1]).map(renameId),
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
					stitchedAggregate.transaction.transactions = [transactions[index + 1], transactions[index + 2]].map(renameId);
					return stitchedAggregate;
				}).map(renameId),
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
				return runTransactionsDbTest(
					{ transactions: seedTransactions },
					db => db.transactionsByIds(TransactionGroups.confirmed, [documentId]),
					transactions => assertEqualDocuments([renameId(seedTransactions[4])], transactions)
				);
			});
		});

		describe('translates group to collection name', () => {
			const validObjectId = test.db.createObjectId(10);
			const validHash = '112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00';

			const runTransactionsByIdTest = (dbCall, param, group, collection) => {
				it(group, () => {
					// Arrange:
					const transactionsByIdsImplStub = sinon.stub(CatapultDb.prototype, 'transactionsByIdsImpl').returns('');
					const db = new CatapultDb(Object.assign({ networkId: Mijin_Test_Network }, DefaultPagingOptions));

					// Act
					db[dbCall](group, [param]);

					// Assert
					expect(transactionsByIdsImplStub.calledOnce).to.equal(true);
					expect(transactionsByIdsImplStub.firstCall.args[0]).to.equal(collection);
					transactionsByIdsImplStub.restore();
				});
			};

			const groupToCollectionName = {
				confirmed: 'transactions',
				unconfirmed: 'unconfirmedTransactions',
				partial: 'partialTransactions'
			};

			describe('transactions by ids', () => {
				Object.keys(groupToCollectionName).forEach(group => {
					runTransactionsByIdTest('transactionsByIds', validObjectId, group, groupToCollectionName[group]);
				});
			});
			describe('transactions by hashes', () => {
				Object.keys(groupToCollectionName).forEach(group => {
					runTransactionsByIdTest('transactionsByHashes', validHash, group, groupToCollectionName[group]);
				});
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

		it('does not promote MongoDb.Long to regular `number` for small enough numbers and ends up returning Long always', () => {
			const dbEntity = {
				transactions: [{
					_id: test.db.createObjectId(55),
					meta: {},
					transaction: { id: Long.fromNumber(23), type: 0x12345, parentId: Long.fromNumber(10) }
				}]
			};
			return runDbTest(
				dbEntity,
				db => db.findNamesByIds([23], 0x12345, { id: 'id', name: 'name', parentId: 'parentId' }),
				tuples => {
					expect(tuples[0].parentId instanceof Long).to.be.equal(true);
				}
			);
		});
	});

	describe('query paged documents', () => {
		const account1 = {
			publicKey: test.random.publicKey(),
			address: keyToAddress(test.random.publicKey())
		};
		const account2 = {
			publicKey: test.random.publicKey(),
			address: keyToAddress(test.random.publicKey())
		};
		const { createObjectId } = test.db;
		const sortConditions = { _id: 1 };
		const options = { pageSize: 10, pageNumber: 1 };

		describe('can return empty result', () => {
			it('empty db', () => {
				// Arrange
				const conditions = {};

				// Act + Assert:
				return runDbTest(
					{},
					db => db.queryPagedDocuments(conditions, [], sortConditions, 'accounts', options),
					page => {
						expect(page.data).to.deep.equal([]);
						expect(page.pagination).to.deep.equal({ pageNumber: 1, pageSize: options.pageSize });
					}
				);
			});

			it('conditions produces empty result', () => {
				// Arrange
				const dbAccounts = () => [
					{ _id: createObjectId(10), account: { addressHeight: 10 } },
					{ _id: createObjectId(30), account: { addressHeight: 30 } }
				];
				const conditions = { 'account.addressHeight': 20 };

				// Act + Assert:
				return runDbTest(
					{ accounts: dbAccounts() },
					db => db.queryPagedDocuments(conditions, [], sortConditions, 'accounts', options),
					page => {
						expect(page.data).to.deep.equal([]);
						expect(page.pagination).to.deep.equal({ pageNumber: 1, pageSize: options.pageSize });
					}
				);
			});
		});

		describe('respects query conditions', () => {
			// Arrange:
			const accounts = () => ([
				{ _id: createObjectId(10), account: { address: account1.address, addressHeight: 10 } },
				{ _id: createObjectId(20), account: { address: account2.address, addressHeight: 20 } },
				{ _id: createObjectId(30), account: { address: account2.address, addressHeight: 30 } }
			]);

			it('no conditions', () => {
				const conditions = {};

				// Act + Assert:
				return runDbTest(
					{ accounts: accounts() },
					db => db.queryPagedDocuments(conditions, [], sortConditions, 'accounts', options),
					page => {
						expect(page.data.length).to.equal(3);
						expect(page.data[0].id).to.deep.equal(createObjectId(10));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(30));
					}
				);
			});

			it('one condition', () => {
				const conditions = { 'account.addressHeight': 20 };

				// Act + Assert:
				return runDbTest(
					{ accounts: accounts() },
					db => db.queryPagedDocuments(conditions, [], sortConditions, 'accounts', options),
					page => {
						expect(page.data.length).to.equal(1);
						expect(page.data[0].id).to.deep.equal(createObjectId(20));
					}
				);
			});

			it('multiple conditions', () => {
				const conditions = {
					'account.address': account2.address,
					'account.addressHeight': { $gt: 20 }
				};

				// Act + Assert:
				return runDbTest(
					{ accounts: accounts() },
					db => db.queryPagedDocuments(conditions, [], sortConditions, 'accounts', options),
					page => {
						expect(page.data.length).to.equal(1);
						expect(page.data[0].id).to.deep.equal(createObjectId(30));
					}
				);
			});
		});

		describe('renames _id to id', () => {
			// Arrange:
			const blocks = () => ([
				{
					_id: createObjectId(10),
					meta: {
						hash: 0x0100,
						transactionsCount: 10
					},
					block: {
						version: 1,
						type: 2
					}
				}
			]);

			it('id is in the query result and _id is not', () =>
				// Act + Assert:
				runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, [], sortConditions, 'blocks', options),
					page => {
						expect(page.data[0]._id).to.equal(undefined);
						expect(page.data[0].id).to.deep.equal(createObjectId(10));
					}
				));

			it('_id is gone when explicitly removed and id isn\'t present in the result either', () =>
				// Act + Assert:
				runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, ['_id'], sortConditions, 'blocks', options),
					page => {
						expect(page.data[0]._id).to.equal(undefined);
						expect(page.data[0].id).to.equal(undefined);
					}
				));
		});

		describe('removed fields', () => {
			// Arrange:
			const blocks = () => ([
				{
					_id: createObjectId(10),
					meta: {
						hash: 0x0100,
						transactionsCount: 10
					},
					block: {
						version: 1,
						type: 2
					}
				}
			]);

			it('does not remove fields if no supplied fields to remove', () => {
				const removedFields = [];

				// Act + Assert:
				return runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, removedFields, sortConditions, 'blocks', options),
					page => {
						expect(page.data.length).to.equal(1);
						expect(page.data[0]).to.deep.equal({
							id: createObjectId(10),
							meta: { hash: 0x0100, transactionsCount: 10 },
							block: { version: 1, type: 2 }
						});
					}
				);
			});

			it('removes supplied fields', () => {
				const removedFields = ['meta.hash', 'block.version'];

				// Act + Assert:
				return runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, removedFields, sortConditions, 'blocks', options),
					page => {
						expect(page.data.length).to.equal(1);
						expect(page.data[0]).to.deep.equal({
							id: createObjectId(10),
							meta: { transactionsCount: 10 },
							block: { type: 2 }
						});
					}
				);
			});
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const blocks = () => ([
				{ _id: createObjectId(10), meta: { transactionsCount: 1 }, block: { version: 3, type: 2 } },
				{ _id: createObjectId(20), meta: { transactionsCount: 2 }, block: { version: 2, type: 1 } },
				{ _id: createObjectId(30), meta: { transactionsCount: 3 }, block: { version: 1, type: 3 } }
			]);

			it('direction ascending', () =>
				// Act + Assert:
				runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, [], { _id: 1 }, 'blocks', options),
					page => {
						expect(page.data.length).to.equal(3);
						expect(page.data[0].id).to.deep.equal(createObjectId(10));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(30));
					}
				));

			it('direction descending', () =>
				// Act + Assert:
				runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, [], { _id: -1 }, 'blocks', options),
					page => {
						expect(page.data.length).to.equal(3);
						expect(page.data[0].id).to.deep.equal(createObjectId(30));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(10));
					}
				));

			it('sort field', () =>
				// Act + Assert:
				runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, [], { 'block.type': 1 }, 'blocks', options),
					page => {
						expect(page.data.length).to.equal(3);
						expect(page.data[0].id).to.deep.equal(createObjectId(20));
						expect(page.data[1].id).to.deep.equal(createObjectId(10));
						expect(page.data[2].id).to.deep.equal(createObjectId(30));
					}
				));
		});

		describe('uses provided collection', () => {
			// Arrange:
			const accounts = () => ([{ _id: createObjectId(10), account: { address: account1.address, addressHeight: 10 } }]);
			const blocks = () => ([{ _id: createObjectId(20), meta: { transactionsCount: 1 }, block: { version: 3, type: 2 } }]);
			const transactions = () => ([{ _id: createObjectId(30), meta: { height: 1 }, transaction: { type: 3 } }]);

			it('respects collection', () =>
				// Act + Assert:
				runDbTest(
					{ accounts: accounts(), blocks: blocks(), transactions: transactions() },
					db => db.queryPagedDocuments({}, [], sortConditions, 'blocks', options),
					page => {
						expect(page.data.length).to.equal(1);
						expect(page.data[0].id).to.deep.equal(createObjectId(20));
					}
				));
		});

		describe('options', () => {
			const blocks = numBlocks => {
				const resultBlocks = [];
				for (let i = 0; numBlocks > i; i++)
					resultBlocks.push({ _id: createObjectId(i), meta: { hash: 0x0100 }, block: { type: 1 } });

				return resultBlocks;
			};

			describe('respects page size', () => {
				it('page size: 25', () => {
					const pageSize = 25;

					// Act + Assert:
					return runDbTest(
						{ blocks: blocks(25 + 10) },
						db => db.queryPagedDocuments({}, [], { id: 1 }, 'blocks', { pageSize, pageNumber: 1 }),
						page => {
							expect(page.data.length).to.equal(25);
							expect(page.pagination).to.deep.equal({ pageNumber: 1, pageSize: 25 });
						}
					);
				});

				it('less results than a page size', () => {
					const pageSize = 25;

					// Act + Assert:
					return runDbTest(
						{ blocks: blocks(10) },
						db => db.queryPagedDocuments({}, [], { id: 1 }, 'blocks', { pageSize, pageNumber: 1 }),
						page => {
							expect(page.data.length).to.equal(10);
							expect(page.pagination).to.deep.equal({ pageNumber: 1, pageSize: 25 });
						}
					);
				});
			});

			describe('respects page number', () => {
				const runPageNumberTest = (pageNumber, expectedNumberOfElements) => {
					it(`page number: ${pageNumber}`, () =>
						// Act + Assert:
						runDbTest(
							{ blocks: blocks(12) },
							db => db.queryPagedDocuments(
								{},
								[],
								{ id: 1 },
								'blocks',
								{ pageSize: 10, pageNumber }
							),
							page => {
								expect(page.data.length).to.equal(expectedNumberOfElements);
								expect(page.pagination).to.deep.equal({ pageNumber, pageSize: 10 });
							}
						));
				};

				runPageNumberTest(1, 10);
				runPageNumberTest(2, 2);
				runPageNumberTest(3, 0);
			});
		});

		describe('does not promote MongoDb.Long to regular `number` for small enough numbers', () => {
			// Arrange:
			const blocks = () => ([{ _id: createObjectId(10), block: { height: Long.fromNumber(10) } }]);

			it('returns long', () =>
				// Act + Assert:
				runDbTest(
					{ blocks: blocks() },
					db => db.queryPagedDocuments({}, [], sortConditions, 'blocks', options),
					page => {
						expect(page.data[0].block.height instanceof Long).to.be.equal(true);
					}
				));
		});
	});

	describe('transactions', () => {
		const account1 = { publicKey: test.random.publicKey() };
		account1.address = keyToAddress(account1.publicKey);
		const account2 = { publicKey: test.random.publicKey() };
		account2.address = keyToAddress(account2.publicKey);
		const account3 = { publicKey: test.random.publicKey() };
		account3.address = keyToAddress(account3.publicKey);

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const { createObjectId } = test.db;

		const createTransaction = (objectId, addresses, height, signerPublicKey, recipientAddress, type, mosaics) => ({
			_id: createObjectId(objectId),
			meta: {
				height,
				addresses: addresses.map(a => Buffer.from(a))
			},
			transaction: {
				signerPublicKey: signerPublicKey ? Buffer.from(signerPublicKey) : undefined,
				recipientAddress: recipientAddress ? Buffer.from(recipientAddress) : undefined,
				mosaics,
				type
			}
		});

		const createInnerTransaction = (objectId, aggregateId, signerPublicKey, recipientAddress, type) => ({
			_id: createObjectId(objectId),
			meta: { aggregateId: createObjectId(aggregateId) },
			transaction: {
				signerPublicKey: signerPublicKey ? Buffer.from(signerPublicKey) : undefined,
				recipientAddress: recipientAddress ? Buffer.from(recipientAddress) : undefined,
				type
			}
		});

		const runTestAndVerifyIds = (dbTransactions, filters, options, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runDbTest(
				{ transactions: dbTransactions },
				db => db.transactions(TransactionGroups.confirmed, filters, options),
				transactionsPage => {
					const returnedIds = transactionsPage.data.map(t => t.id);
					expect(transactionsPage.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbTransactions = [
				createTransaction(10, [account1.address], 123, account1.publicKey, account2.address, EntityType.transfer)
			];

			// Act + Assert:
			return runDbTest(
				{ transactions: dbTransactions },
				db => db.transactions(TransactionGroups.confirmed, {}, paginationOptions),
				page => {
					const expected_keys = ['meta', 'transaction', 'id'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('does not expose private meta.addresses field', () => {
			// Arrange:
			const dbTransactions = [
				createTransaction(10, [account1.address], 1, 0, 0, 0)
			];

			// Act + Assert:
			return runDbTest(
				{ transactions: dbTransactions },
				db => db.transactions(TransactionGroups.confirmed, {}, paginationOptions),
				transactionsPage => {
					expect(transactionsPage.data[0].meta.addresses).to.equal(undefined);
				}
			);
		});

		it('if address is provided signerPublicKey and recipientAddress are omitted', () => {
			// Arrange:
			const dbTransactions = [
				createTransaction(10, [account1.address], 1),
				createTransaction(20, [account1.address], 1, account1.publicKey),
				createTransaction(30, [account1.address], 1, account2.publicKey),
				createTransaction(40, [account1.address], 1, account1.publicKey, account1.address),
				createTransaction(50, [account1.address], 1, account2.publicKey, account2.address),
				createTransaction(60, [account2.address], 1)
			];

			const filters = {
				address: account1.address,
				signerPublicKey: account1.publicKey,
				recipientAddress: account1.address
			};

			// Act + Assert:
			return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 20, 30, 40, 50]);
		});

		it('ignores inner aggregate transactions in the results', () => {
			// Arrange:
			const dbTransactions = [
				// Aggregate
				createTransaction(10, [], 1, 0, 0, EntityType.aggregateComplete),
				createInnerTransaction(100, 30, 0, 0, EntityType.mosaicDefinition),
				createInnerTransaction(200, 30, 0, 0, EntityType.mosaicSupplyChange),

				createTransaction(20, [], 1, 0, 0, EntityType.aggregateBonded),
				createInnerTransaction(300, 30, 0, 0, EntityType.transfer),
				createInnerTransaction(400, 30, 0, 0, EntityType.mosaicDefinition),

				createTransaction(30, [], 1, 0, 0, EntityType.aggregateComplete),
				createInnerTransaction(500, 30, 0, 0, EntityType.registerNamespace)
			];

			const filters = {
				transactionTypes: [EntityType.transfer, EntityType.mosaicDefinition, EntityType.aggregateComplete]
			};

			// Act + Assert:
			return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 30]);
		});

		it('returns correct transactions when no filters are provided', () => {
			// Arrange:
			const dbTransactions = [
				createTransaction(10, [account1.address], 1),
				createTransaction(20, [account1.address], 1, account1.publicKey),
				createTransaction(30, [account1.address], 1, account1.publicKey, account1.address),
				createTransaction(40, [account1.address], 1, account1.publicKey, account1.address, EntityType.transfer)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbTransactions, {}, paginationOptions, [10, 20, 30, 40]);
		});

		it('all the provided filters are taken into account', () => {
			// Arrange:
			const dbTransactions = [
				createTransaction(10, [account1.address], 1),
				createTransaction(20, [account1.address], 1),
				createTransaction(30, [account1.address], 1, account1.publicKey),
				createTransaction(40, [account1.address], 1, account1.publicKey, account1.address),
				createTransaction(50, [account1.address], 1, account1.publicKey, account1.address, EntityType.transfer),
				createTransaction(
					60, [account1.address], 1, account1.publicKey, account1.address, EntityType.transfer, [{ id: 10, amount: 100 }]
				),
				createTransaction(
					70, [account1.address], 1, account1.publicKey, account1.address, EntityType.transfer, [{ id: 10, amount: 200 }]
				),
				createTransaction(
					80, [account1.address], 1, account1.publicKey, account1.address, EntityType.transfer, [{ id: 10, amount: 300 }]
				)
			];

			const filters = {
				height: 1,
				signerPublicKey: account1.publicKey,
				recipientAddress: account1.address,
				transactionTypes: [EntityType.transfer],
				transferMosaicId: 10,
				fromTransferAmount: 101,
				toTransferAmount: 299
			};

			// Act + Assert:
			return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [70]);
		});

		describe('respects offset', () => {
			// Arrange:
			const dbTransactions = () => [
				createTransaction(10, [], 20),
				createTransaction(20, [], 30),
				createTransaction(30, [], 10)
			];
			const options = {
				pageSize: 10,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1,
				offset: createObjectId(20)
			};

			it('gt', () => {
				options.sortDirection = 1;

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions(), {}, options, [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions(), {}, options, [10]);
			});
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbTransactions = () => [
				createTransaction(10, [], 20),
				createTransaction(20, [], 30),
				createTransaction(30, [], 10)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runDbTest(
					{ transactions: dbTransactions() },
					db => db.transactions(TransactionGroups.confirmed, [], options),
					transactionsPage => {
						expect(transactionsPage.data[0].id).to.deep.equal(createObjectId(10));
						expect(transactionsPage.data[1].id).to.deep.equal(createObjectId(20));
						expect(transactionsPage.data[2].id).to.deep.equal(createObjectId(30));
					}
				);
			});

			it('direction descending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: -1
				};

				// Act + Assert:
				return runDbTest(
					{ transactions: dbTransactions() },
					db => db.transactions(TransactionGroups.confirmed, [], options),
					transactionsPage => {
						expect(transactionsPage.data[0].id).to.deep.equal(createObjectId(30));
						expect(transactionsPage.data[1].id).to.deep.equal(createObjectId(20));
						expect(transactionsPage.data[2].id).to.deep.equal(createObjectId(10));
					}
				);
			});

			it('sort field', () => {
				const queryPagedDocumentsSpy = sinon.spy(CatapultDb.prototype, 'queryPagedDocuments');
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runDbTest(
					{ transactions: dbTransactions() },
					db => db.transactions(TransactionGroups.confirmed, [], options),
					() => {
						expect(queryPagedDocumentsSpy.calledOnce).to.equal(true);
						expect(Object.keys(queryPagedDocumentsSpy.firstCall.args[2])[0]).to.equal('_id');
						queryPagedDocumentsSpy.restore();
					}
				);
			});
		});

		describe('correctly applies each filter:', () => {
			it('height', () => {
				// Arrange:
				const dbTransactions = [
					createTransaction(10, [], 5),
					createTransaction(20, [], 10),
					createTransaction(30, [], 15)
				];

				const filters = { height: 10 };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [20]);
			});

			it('fromHeight', () => {
				// Arrange:
				const dbTransactions = [
					createTransaction(10, [], 5),
					createTransaction(20, [], 10),
					createTransaction(30, [], 15)
				];

				const filters = { fromHeight: 10 };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [20, 30]);
			});

			it('toHeight', () => {
				// Arrange:
				const dbTransactions = [
					createTransaction(10, [], 5),
					createTransaction(20, [], 10),
					createTransaction(30, [], 15)
				];

				const filters = { toHeight: 10 };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 20]);
			});

			it('address', () => {
				// Arrange:
				const dbTransactions = [
					createTransaction(10, [account1.address], 1),
					createTransaction(20, [account2.address], 1),
					createTransaction(30, [account3.address], 1),
					createTransaction(40, [account2.address, account1.address], 1),
					createTransaction(50, [account3.address, account1.address], 1)
				];

				const filters = { address: account1.address };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 40, 50]);
			});

			it('signerPublicKey', () => {
				// Arrange:
				const dbTransactions = [
					// Non aggregate
					createTransaction(10, [], 1, account1.publicKey),
					createTransaction(20, [], 1, account2.publicKey),

					// Aggregate
					createTransaction(30, [], 1, account1.publicKey),
					createInnerTransaction(100, 30, account2.publicKey),
					createInnerTransaction(200, 30, account2.publicKey),

					createTransaction(40, [], 1, account2.publicKey),
					createInnerTransaction(300, 40, account1.publicKey),
					createInnerTransaction(400, 40, account2.publicKey),

					createTransaction(50, [], 1, account2.publicKey),
					createInnerTransaction(500, 50, account2.publicKey)
				];

				const filters = { signerPublicKey: account1.publicKey };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 30]);
			});

			it('recipientAddress', () => {
				// Arrange:
				const dbTransactions = [
					// Non aggregate
					createTransaction(10, [], 1, 0, account1.address),
					createTransaction(20, [], 1, 0, account2.address),

					// Aggregate
					createTransaction(30, [], 1, 0, account1.address),
					createInnerTransaction(100, 30, 0, account2.address),
					createInnerTransaction(200, 30, 0, account2.address),

					createTransaction(40, [], 1, 0, account2.address),
					createInnerTransaction(300, 40, 0, account1.address),
					createInnerTransaction(400, 40, 0, account2.address),

					createTransaction(50, [], 1, 0, account2.address),
					createInnerTransaction(500, 50, 0, account2.address)
				];

				const filters = { recipientAddress: account1.address };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 30]);
			});

			it('transactionTypes', () => {
				// Arrange:
				const dbTransactions = [
					// Non aggregate
					createTransaction(10, [], 1, 0, 0, EntityType.transfer),
					createTransaction(20, [], 1, 0, 0, EntityType.accountLink),

					// Aggregate
					createTransaction(30, [], 1, 0, 0, EntityType.aggregateBonded),
					createInnerTransaction(100, 30, 0, 0, EntityType.mosaicDefinition),
					createInnerTransaction(200, 30, 0, 0, EntityType.mosaicSupplyChange),

					createTransaction(40, [], 1, 0, 0, EntityType.aggregateComplete),
					createInnerTransaction(300, 40, 0, 0, EntityType.transfer),
					createInnerTransaction(400, 40, 0, 0, EntityType.transfer),

					createTransaction(50, [], 1, 0, 0, EntityType.aggregateBonded),
					createInnerTransaction(500, 50, 0, 0, EntityType.registerNamespace),
					createInnerTransaction(600, 50, 0, 0, EntityType.aliasAddress)
				];

				const filters = {
					transactionTypes: [EntityType.mosaicDefinition, EntityType.aggregateComplete, EntityType.transfer]
				};

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 40]);
			});

			it('transferMosaicId', () => {
				// Arrange:
				const dbTransactions = [
					// Non aggregate
					createTransaction(10, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 1 }]),
					createTransaction(20, [], 1, 0, 0, EntityType.transfer, [{ id: 20, amount: 1 }]),
					createTransaction(30, [], 1, 0, 0, EntityType.accountMetadata, [{ id: 10, amount: 1 }, { id: 20, amount: 1 }])
				];

				const filters = { transferMosaicId: 20 };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [20, 30]);
			});

			it('fromTransferAmount', () => {
				// Arrange:
				const dbTransactions = [
					// Non aggregate
					createTransaction(10, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 5 }]),
					createTransaction(20, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 6 }]),
					createTransaction(30, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 7 }])
				];

				const filters = { transferMosaicId: 10, fromTransferAmount: 6 };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [20, 30]);
			});

			it('toTransferAmount', () => {
				// Arrange:
				const dbTransactions = [
					// Non aggregate
					createTransaction(10, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 5 }]),
					createTransaction(20, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 6 }]),
					createTransaction(30, [], 1, 0, 0, EntityType.transfer, [{ id: 10, amount: 7 }])
				];

				const filters = { transferMosaicId: 10, toTransferAmount: 6 };

				// Act + Assert:
				return runTestAndVerifyIds(dbTransactions, filters, paginationOptions, [10, 20]);
			});

			describe('group', () => {
				// Arrange:
				const dbTransactions = () => ({
					transactions: [createTransaction(10, [], 1)],
					partialTransactions: [createTransaction(20, [], 1)],
					unconfirmedTransactions: [createTransaction(30, [], 1)]
				});

				const runGroupTest = (group, expectedIds) => {
					it(`group: ${group}`, () => {
						const expectedObjectIds = expectedIds.map(id => createObjectId(id));

						return runDbTest(
							dbTransactions(),
							db => db.transactions(group, {}, paginationOptions),
							transactionsPage => {
								const returnedIds = transactionsPage.data.map(t => t.id);
								expect(transactionsPage.data.length).to.equal(expectedObjectIds.length);
								expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
							}
						);
					});
				};

				runGroupTest(TransactionGroups.confirmed, [10]);
				runGroupTest(TransactionGroups.partial, [20]);
				runGroupTest(TransactionGroups.unconfirmed, [30]);

				it('defaults to confirmed', () =>
					// Act + Assert:
					runDbTest(
						dbTransactions(),
						db => db.transactions(TransactionGroups.confirmed, {}, paginationOptions),
						transactionsPage => {
							expect(transactionsPage.data.length).to.equal(1);
							expect(transactionsPage.data[0].id).to.deep.equal(createObjectId(10));
						}
					));
			});

			describe('embedded', () => {
				// Arrange:
				const dbTransactions = () => [
					// Non aggregate
					createTransaction(10, [], 1),

					// Aggregate
					createTransaction(20, [], 1),
					createInnerTransaction(100, 20)
				];

				const runEmbeddedTest = (embedded, expectedIds) =>
					it(`embedded: ${embedded}`, () =>
						// Act + Assert:
						runTestAndVerifyIds(dbTransactions(), { embedded }, paginationOptions, expectedIds));

				runEmbeddedTest(true, [10, 20, 100]);
				runEmbeddedTest(false, [10, 20]);

				it('defaults to false', () =>
					// Act + Assert:
					runTestAndVerifyIds(dbTransactions(), {}, paginationOptions, [10, 20]));
			});
		});
	});

	describe('accounts', () => {
		const addressTest1 = address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ');
		const addressTest2 = address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA');
		const addressTest3 = address.stringToAddress('SAAM2O7SSJ2A7AU3DZJMSTTRFZT5TFDPQ3ZIIJX');
		const addressTest4 = address.stringToAddress('SAMZMPX33DFIIVOCNJYMF5KJTGLAEVNKHHFROLX');
		const mosaicIdTest1 = Long.fromNumber(12345678);
		const mosaicIdTest2 = Long.fromNumber(87654321);

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const { createObjectId } = test.db;

		const createAccount = (objectId, accountAddress, mosaics, importances) => ({
			_id: createObjectId(objectId),
			account: {
				address: accountAddress ? Buffer.from(accountAddress) : undefined,
				mosaics,
				importances: importances || []
			}
		});

		const runTestAndVerifyIds = (dbAccounts, accountAddress, mosaicId, options, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runDbTest(
				{ accounts: dbAccounts },
				db => db.accounts(accountAddress, mosaicId, options),
				page => {
					const returnedIds = page.data.map(t => t.id);
					expect(page.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		const runAccountsFilteredByAddressTest = pagination => {
			it('returns accounts filtered by accountAddress', () => {
				// Arrange:
				const dbAccounts = [
					createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
					createAccount(20, addressTest2, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
					createAccount(30, addressTest3, [])
				];

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts, addressTest2, undefined, pagination, [20]);
			});
		};

		const runAccountsFilteredByMosaicIdTest = pagination => {
			it('returns accounts filtered by mosaicId', () => {
				// Arrange:
				const dbAccounts = [
					createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
					createAccount(20, addressTest1, [{ id: mosaicIdTest2, amount: Long.fromNumber(1) }])
				];

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts, undefined, mosaicIdTest2, pagination, [20]);
			});
		};

		const runImportancesDbTest = (dbAccounts, pagination, value, height) =>
			runDbTest(
				{ accounts: dbAccounts },
				db => db.accounts(undefined, undefined, pagination),
				page => {
					expect(page.data[0].account.importance).to.deep.equal(Long.fromNumber(value));
					expect(page.data[0].account.importanceHeight).to.deep.equal(Long.fromNumber(height));
					expect(page.data[0].account.importances).to.equal(undefined);
				}
			);

		it('returns expected structure', () => {
			// Arrange:
			const dbAccounts = [createAccount(10, addressTest1, [])];

			// Act + Assert:
			return runDbTest(
				{ accounts: dbAccounts },
				db => db.accounts(undefined, undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'account'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('returns empty for unknown accountAddress', () => {
			// Arrange:
			const dbAccounts = [createAccount(10, addressTest1, [])];

			// Act + Assert:
			return runTestAndVerifyIds(dbAccounts, addressTest2, undefined, paginationOptions, []);
		});

		it('returns empty for unknown mosaicId', () => {
			// Arrange:
			const dbAccounts = [
				createAccount(10, addressTest1, [
					{ id: mosaicIdTest1, amount: Long.fromNumber(1) }
				])
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbAccounts, undefined, mosaicIdTest2, paginationOptions, []);
		});

		runAccountsFilteredByAddressTest(paginationOptions);

		runAccountsFilteredByMosaicIdTest(paginationOptions);

		it('returns all accounts if no accountAddress or mosaicId are provided', () => {
			// Arrange:
			const dbAccounts = [
				createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
				createAccount(20, addressTest2, [{ id: mosaicIdTest2, amount: Long.fromNumber(1) }]),
				createAccount(30, addressTest3, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
				createAccount(40, addressTest4, [{ id: mosaicIdTest2, amount: Long.fromNumber(1) }])
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbAccounts, undefined, undefined, paginationOptions, [10, 20, 30, 40]);
		});

		describe('picks top importance', () => {
			it('no importances', () => {
				// Arrange:
				const dbAccounts = [createAccount(10, addressTest1, [], [])];

				// Act + Assert:
				return runImportancesDbTest(dbAccounts, paginationOptions, 0, 0);
			});

			it('one importance', () => {
				// Arrange:
				const importances = [
					{ value: Long.fromNumber(100), height: Long.fromNumber(10) }
				];
				const dbAccounts = [createAccount(10, addressTest1, [], importances)];

				// Act + Assert:
				return runImportancesDbTest(dbAccounts, paginationOptions, 100, 10);
			});

			it('multiple importances', () => {
				// Arrange:
				const importances = [
					{ value: Long.fromNumber(100), height: Long.fromNumber(10) },
					{ value: Long.fromNumber(200), height: Long.fromNumber(20) }
				];
				const dbAccounts = [createAccount(10, addressTest1, [], importances)];

				// Act + Assert:
				return runImportancesDbTest(dbAccounts, paginationOptions, 100, 10);
			});
		});

		describe('respects offset', () => {
			// Arrange:
			const dbAccounts = () => ([
				createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
				createAccount(20, addressTest2, [{ id: mosaicIdTest1, amount: Long.fromNumber(2) }]),
				createAccount(30, addressTest3, [{ id: mosaicIdTest1, amount: Long.fromNumber(3) }])
			]);
			const options = {
				pageSize: 10,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1,
				offset: createObjectId(20)
			};

			it('gt', () => {
				options.sortDirection = 1;

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts(), undefined, undefined, options, [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts(), undefined, undefined, options, [10]);
			});
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbAccounts = () => ([
				createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
				createAccount(20, addressTest1, [{ id: mosaicIdTest2, amount: Long.fromNumber(1) }]),
				createAccount(30, addressTest2, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }])
			]);

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runDbTest(
					{ accounts: dbAccounts() },
					db => db.accounts(undefined, undefined, options),
					page => {
						expect(page.data[0].id).to.deep.equal(createObjectId(10));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(30));
					}
				);
			});

			it('direction descending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: -1
				};

				// Act + Assert:
				return runDbTest(
					{ accounts: dbAccounts() },
					db => db.accounts(undefined, undefined, options),
					page => {
						expect(page.data[0].id).to.deep.equal(createObjectId(30));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(10));
					}
				);
			});

			it('sort field', () => {
				const queryPagedDocumentsSpy = sinon.spy(CatapultDb.prototype, 'queryPagedDocuments');
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runDbTest(
					{ accounts: dbAccounts() },
					db => db.accounts(undefined, undefined, options),
					() => {
						expect(queryPagedDocumentsSpy.calledOnce).to.equal(true);
						expect(Object.keys(queryPagedDocumentsSpy.firstCall.args[2])[0]).to.equal('_id');
						queryPagedDocumentsSpy.restore();
					}
				);
			});
		});

		describe('when sorting by balance', () => {
			const paginationOptionsBalanceSorting = {
				pageSize: 10,
				pageNumber: 1,
				sortField: 'balance',
				sortDirection: -1
			};

			it('returns expected structure', () => {
				// Arrange:
				const dbAccounts = [createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }])];

				// Act + Assert:
				return runDbTest(
					{ accounts: dbAccounts },
					db => db.accounts(undefined, undefined, paginationOptionsBalanceSorting),
					page => {
						const expected_keys = ['id', 'account'];
						expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
					}
				);
			});

			it('returns empty for unknown accountAddress', () => {
				// Arrange:
				const dbAccounts = [createAccount(10, addressTest1, [])];

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts, addressTest2, undefined, paginationOptionsBalanceSorting, []);
			});

			it('returns empty for unknown mosaicId', () => {
				// Arrange:
				const dbAccounts = [
					createAccount(10, addressTest1, [
						{ id: mosaicIdTest1, amount: Long.fromNumber(1) }
					])
				];

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts, undefined, mosaicIdTest2, paginationOptionsBalanceSorting, []);
			});

			runAccountsFilteredByAddressTest(paginationOptionsBalanceSorting);

			runAccountsFilteredByMosaicIdTest(paginationOptionsBalanceSorting);

			it('returns all accounts if no accountAddress or mosaicId are provided', () => {
				// Arrange:
				const dbAccounts = [
					createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
					createAccount(20, addressTest2, [{ id: mosaicIdTest2, amount: Long.fromNumber(1) }]),
					createAccount(30, addressTest3, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
					createAccount(40, addressTest4, [{ id: mosaicIdTest2, amount: Long.fromNumber(1) }])
				];

				// Act + Assert:
				return runTestAndVerifyIds(dbAccounts, undefined, undefined, paginationOptionsBalanceSorting, [10, 20, 30, 40]);
			});

			describe('picks top importance', () => {
				it('no importances', () => {
					// Arrange:
					const dbAccounts = [createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }], [])];

					// Act + Assert:
					return runImportancesDbTest(dbAccounts, paginationOptionsBalanceSorting, 0, 0);
				});

				it('one importance', () => {
					// Arrange:
					const importances = [
						{ value: Long.fromNumber(100), height: Long.fromNumber(10) }
					];
					const dbAccounts = [createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }], importances)];

					// Act + Assert:
					return runImportancesDbTest(dbAccounts, paginationOptionsBalanceSorting, 100, 10);
				});

				it('multiple importances', () => {
					// Arrange:
					const importances = [
						{ value: Long.fromNumber(100), height: Long.fromNumber(10) },
						{ value: Long.fromNumber(200), height: Long.fromNumber(20) }
					];
					const dbAccounts = [createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }], importances)];

					// Act + Assert:
					return runImportancesDbTest(dbAccounts, paginationOptionsBalanceSorting, 100, 10);
				});
			});

			describe('respects offset', () => {
				// Arrange:
				const dbAccounts = () => ([
					createAccount(10, addressTest1, [{ id: mosaicIdTest1, amount: Long.fromNumber(1) }]),
					createAccount(20, addressTest2, [{ id: mosaicIdTest1, amount: Long.fromNumber(2) }]),
					createAccount(30, addressTest3, [{ id: mosaicIdTest1, amount: Long.fromNumber(3) }])
				]);
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'balance',
					sortDirection: 1,
					offset: Long.fromNumber(2)
				};

				it('gt', () => {
					options.sortDirection = 1;

					// Act + Assert:
					return runTestAndVerifyIds(dbAccounts(), undefined, mosaicIdTest1, options, [30]);
				});

				it('lt', () => {
					options.sortDirection = -1;

					// Act + Assert:
					return runTestAndVerifyIds(dbAccounts(), undefined, mosaicIdTest1, options, [10]);
				});
			});

			describe('sorts by correct mosaic', () => {
				const seedAccounts = [];
				seedAccounts.push(createAccount(
					1,
					test.random.address(),
					[
						{ id: Long.fromNumber(22), amount: Long.fromNumber(1) },
						{ id: Long.fromNumber(33), amount: Long.fromNumber(5) },
						{ id: Long.fromNumber(44), amount: Long.fromNumber(3) }
					]
				));
				seedAccounts.push(createAccount(
					2,
					test.random.address(),
					[
						{ id: Long.fromNumber(22), amount: Long.fromNumber(7) }
					]
				));
				seedAccounts.push(createAccount(
					3,
					test.random.address(),
					[
						{ id: Long.fromNumber(33), amount: Long.fromNumber(8) }
					]
				));
				seedAccounts.push(createAccount(
					4,
					test.random.address(),
					[
						{ id: Long.fromNumber(44), amount: Long.fromNumber(9) }
					]
				));
				seedAccounts.push(createAccount(
					5,
					test.random.address(),
					[
						{ id: Long.fromNumber(22), amount: Long.fromNumber(4) },
						{ id: Long.fromNumber(33), amount: Long.fromNumber(2) },
						{ id: Long.fromNumber(44), amount: Long.fromNumber(6) }
					]
				));

				const mosaicAmountEquals = (accountsPage, accountIndex, mosaicId, amount) =>
					accountsPage.data[accountIndex].account.mosaics
						.find(mosaic => mosaic.id.equals(Long.fromNumber(mosaicId))).amount.equals(Long.fromNumber(amount));

				it('asc', () =>
					runDbTest(
						{ accounts: seedAccounts },
						db => db.accounts(undefined, 33, {
							sortField: 'balance',
							sortDirection: 1,
							pageNumber: 1,
							pageSize: 10
						}),
						accountsPage => {
							expect(mosaicAmountEquals(accountsPage, 0, 33, 2)).to.equal(true);
							expect(mosaicAmountEquals(accountsPage, 1, 33, 5)).to.equal(true);
							expect(mosaicAmountEquals(accountsPage, 2, 33, 8)).to.equal(true);
						}
					));

				it('desc', () =>
					runDbTest(
						{ accounts: seedAccounts },
						db => db.accounts(undefined, 33, {
							sortField: 'balance',
							sortDirection: -1,
							pageNumber: 1,
							pageSize: 10
						}),
						accountsPage => {
							expect(mosaicAmountEquals(accountsPage, 0, 33, 8)).to.equal(true);
							expect(mosaicAmountEquals(accountsPage, 1, 33, 5)).to.equal(true);
							expect(mosaicAmountEquals(accountsPage, 2, 33, 2)).to.equal(true);
						}
					));
			});
		});
	});

	describe('accounts by ids', () => {
		const publicKey = test.random.publicKey();
		const decodedAddress = keyToAddress(publicKey);
		const publicKeyUnknown = test.random.publicKey();
		const decodedAddressUnknown = keyToAddress(publicKeyUnknown);

		const runAccountByIdsDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) => {
			// Arrange:
			const db = new CatapultDb(Object.assign({ networkId: Mijin_Test_Network }, DefaultPagingOptions));

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test', testDbOptions.connectionPoolSize)
				.then(() => test.db.populateDatabase(db, dbEntities))
				.then(() => issueDbCommand(db))
				.then(assertDbCommandResult)
				.then(() => db.close());
		};

		const transformDbAccount = (dbAccountDocument, numImportances) => {
			// the db call should replace importances with the most recent importance and importance height,
			// so update the expected object to match
			const accountWithMetadata = Object.assign({ id: dbAccountDocument._id }, dbAccountDocument);
			delete accountWithMetadata._id;
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
				return runAccountByIdsDbTest(
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

			it('picks top importance', () => {
				const numImportances = 2;
				const seedAccounts = test.db.createAccounts(publicKey, { savePublicKey: true, numMosaics: 1, numImportances });

				// Assert:
				return runAccountByIdsDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([{ address: decodedAddress }]),
					accounts => {
						expect(accounts[0].account.importance).to.deep.equal(Long.fromNumber(numImportances));
						expect(accounts[0].account.importanceHeight).to.deep.equal(Long.fromNumber(numImportances * numImportances));
						expect(accounts[0].account.importances).to.equal(undefined);
					}
				);
			});
		});

		describe('account from public key', () => {
			// note: even if public key is not known in the accounts collection, the call to accountsByIds()
			//       will succeed since the public key is converted to a decoded address.
			addAccountsByIdsTestsForSingleAccountLookup({ publicKey }, { publicKey: publicKeyUnknown });

			it('picks top importance', () => {
				const numImportances = 2;
				const seedAccounts = test.db.createAccounts(publicKey, { savePublicKey: true, numMosaics: 1, numImportances });

				// Assert:
				return runAccountByIdsDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([{ publicKey }]),
					accounts => {
						expect(accounts[0].account.importance).to.deep.equal(Long.fromNumber(numImportances));
						expect(accounts[0].account.importanceHeight).to.deep.equal(Long.fromNumber(numImportances * numImportances));
						expect(accounts[0].account.importances).to.equal(undefined);
					}
				);
			});
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
				runMultipleAccountsByIdsTests((seedAccounts, publicKeys) => runAccountByIdsDbTest(
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
				runMultipleAccountsByIdsTests((seedAccounts, publicKeys) => runAccountByIdsDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([
						{ publicKey: publicKeys[1] },
						{ publicKey: test.random.publicKey() },
						{ address: test.random.address() },
						{ address: keyToAddress(publicKeys[3]) }
					]),
					accounts => expect(accounts).to.deep.equal([1, 3].map(index => transformDbAccount(seedAccounts[index], 1)))
				)));

			it('picks top importance', () => {
				const publicKey1 = test.random.publicKey();
				const publicKey2 = test.random.publicKey();

				const importanceValue = Long.fromNumber(100);
				const importanceHeight = Long.fromNumber(10);

				const seedAccounts = [
					test.db.createAccount(1, publicKey1, true, [], [{ value: importanceValue, height: importanceHeight }, {}]),
					test.db.createAccount(2, publicKey2, true, [], [{ value: importanceValue, height: importanceHeight }, {}])
				];

				// Assert:
				return runAccountByIdsDbTest(
					{ accounts: seedAccounts },
					db => db.accountsByIds([{ publicKey: publicKey1 }, { publicKey: publicKey2 }]),
					accounts => {
						accounts.forEach(account => {
							expect(account.account.importance).to.deep.equal(importanceValue);
							expect(account.account.importanceHeight).to.deep.equal(importanceHeight);
							expect(account.account.importances).to.equal(undefined);
						});
					}
				);
			});
		});
	});

	// region failed transactions

	describe('failed transactions by hashes', () => {
		const runTransactionsByHashesFailedTest = (numSeeds, runTest) => {
			// Arrange:
			const hashes = Array.from(Array(numSeeds), () => test.random.hash());
			const failedTransactionResults = [];
			for (let i = 0; i < numSeeds; ++i)
				failedTransactionResults.push({ status: { hash: new Binary(hashes[i]), validationResult: i } });

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

	// region utils

	describe('utils', () => {
		it('can retrieve account public key from account address', () => {
			// Arrange:
			const accountPublicKeyOne = test.random.publicKey();
			const accountAddressOne = keyToAddress(accountPublicKeyOne);
			const accountPublicKeyTwo = test.random.publicKey();
			const accountAddressTwo = keyToAddress(accountPublicKeyTwo);

			const accountDbEntities = [
				{
					meta: {},
					account: {
						address: new Binary(accountAddressOne),
						publicKey: new Binary(accountPublicKeyOne)
					}
				},
				{
					meta: {},
					account: {
						address: new Binary(accountAddressTwo),
						publicKey: new Binary(accountPublicKeyTwo)
					}
				}
			];

			// Act + Assert:
			return runDbTest(
				{ accounts: accountDbEntities },
				db => db.addressToPublicKey(accountAddressOne),
				accountDbEntity => { expect(accountDbEntity.account.publicKey.buffer.equals(accountPublicKeyOne)).to.be.equal(true); }
			);
		});
	});

	// endregion
});
