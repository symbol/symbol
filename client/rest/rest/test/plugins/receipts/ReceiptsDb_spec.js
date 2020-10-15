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

const CatapultDb = require('../../../src/db/CatapultDb');
const { convertToLong } = require('../../../src/db/dbUtils');
const ReceiptsDb = require('../../../src/plugins/receipts/ReceiptsDb');
const test = require('../../db/utils/dbTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { Binary, Long } = MongoDb;
const { address } = catapult.model;
const { uint64 } = catapult.utils;

describe('receipts db', () => {
	const { createObjectId } = test.db;

	const testAddress1 = address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ');
	const testAddress2 = address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA');

	const paginationOptions = {
		pageSize: 10,
		pageNumber: 1,
		sortField: 'id',
		sortDirection: -1
	};

	const runReceiptsDbTest = (dbEntities, collection, issueDbCommand, assertDbCommandResult) =>
		test.db.runDbTest(dbEntities, collection, db => new ReceiptsDb(db), issueDbCommand, assertDbCommandResult);

	const runTestAndVerifyIds = (dbReceipts, collectionName, dbQuery, expectedIds) => {
		const expectedObjectIds = expectedIds.map(id => createObjectId(id));

		return runReceiptsDbTest(
			dbReceipts,
			collectionName,
			dbQuery,
			receiptsPage => {
				const returnedIds = receiptsPage.data.map(t => t.id);
				expect(receiptsPage.data.length).to.equal(expectedObjectIds.length);
				expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
			}
		);
	};

	describe('transactionStatements', () => {
		const collectionName = 'transactionStatements';

		const createTransactionStatement = (objectId, height, receipts) => ({
			_id: createObjectId(objectId),
			statement: {
				height: Long.fromNumber(height),
				source: { primaryId: 0, secondaryId: 0 },
				receipts
			}
		});

		const createReceipt = fields => ({
			version: 1,
			type: fields.type,
			recipientAddress: fields.recipientAddress ? Buffer.from(fields.recipientAddress) : undefined,
			senderAddress: fields.senderAddress ? Buffer.from(fields.senderAddress) : undefined,
			targetAddress: fields.targetAddress ? Buffer.from(fields.targetAddress) : undefined,
			namespaceId: fields.namespaceId ? convertToLong(fields.namespaceId) : undefined,
			mosaicId: fields.mosaicId ? convertToLong(fields.mosaicId) : undefined
		});

		it('returns expected structure', () => {
			// Arrange:
			const sampleReceipt = createReceipt({ type: 1 });
			const dbReceipts = [createTransactionStatement(10, 100, [sampleReceipt])];
			const filters = {};

			// Act + Assert:
			return runReceiptsDbTest(
				dbReceipts,
				collectionName,
				db => db.transactionStatements(filters, paginationOptions),
				page => {
					const expected_keys = ['id', 'statement'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
					expect(Object.keys(sampleReceipt).sort()).to.deep.equal(
						Object.keys(page.data[0].statement.receipts[0]).sort()
					);
				}
			);
		});

		it('returns empty array for empty height', () => {
			// Arrange:
			const dbReceipts = [createTransactionStatement(10, 100)];
			const filters = { height: 20 };

			// Act + Assert:
			return runTestAndVerifyIds(
				dbReceipts,
				collectionName,
				db => db.transactionStatements(filters, paginationOptions),
				[]
			);
		});

		it('returns all the receipts if no filters are provided', () => {
			// Arrange:
			const dbReceipts = [
				createTransactionStatement(10, 100),
				createTransactionStatement(20, 101),
				createTransactionStatement(30, 102)
			];
			const filters = {};

			// Act + Assert:
			return runTestAndVerifyIds(
				dbReceipts,
				collectionName,
				db => db.transactionStatements(filters, paginationOptions),
				[10, 20, 30]
			);
		});

		describe('all the provided filters are taken into account', () => {
			it('height', () => {
				// Arrange:
				const dbReceipts = [
					createTransactionStatement(10, 100),
					createTransactionStatement(20, 110),
					createTransactionStatement(30, 120)
				];
				const filters = { height: 110 };

				// Act + Assert:
				return runTestAndVerifyIds(
					dbReceipts,
					collectionName,
					db => db.transactionStatements(filters, paginationOptions),
					[20]
				);
			});

			it('receiptType', () => {
				// Arrange:
				const dbReceipts = [
					createTransactionStatement(10, 100, [createReceipt({ type: 1 }), createReceipt({ type: 2 })]),
					createTransactionStatement(20, 110, [createReceipt({ type: 2 }), createReceipt({ type: 3 })]),
					createTransactionStatement(30, 120, [createReceipt({ type: 4 }), createReceipt({ type: 4 })]),
					createTransactionStatement(40, 120, [createReceipt({ type: 5 })]),
					createTransactionStatement(50, 130, [])
				];
				const filters = { receiptType: [2, 5] };

				// Act + Assert:
				return runTestAndVerifyIds(
					dbReceipts,
					collectionName,
					db => db.transactionStatements(filters, paginationOptions),
					[10, 20, 40]
				);
			});

			it('recipientAddress', () => {
				// Arrange:
				const dbReceipts = [
					createTransactionStatement(10, 100, [createReceipt({ recipientAddress: testAddress1 })]),
					createTransactionStatement(20, 110, [
						createReceipt({ recipientAddress: testAddress1 }),
						createReceipt({ recipientAddress: testAddress2 })
					]),
					createTransactionStatement(30, 120, [createReceipt({ recipientAddress: testAddress2 })]),
					createTransactionStatement(40, 130, [])
				];
				const filters = { recipientAddress: testAddress2 };

				// Act + Assert:
				return runTestAndVerifyIds(
					dbReceipts,
					collectionName,
					db => db.transactionStatements(filters, paginationOptions),
					[20, 30]
				);
			});

			it('senderAddress', () => {
				// Arrange:
				const dbReceipts = [
					createTransactionStatement(10, 100, [createReceipt({ senderAddress: testAddress1 })]),
					createTransactionStatement(20, 110, [
						createReceipt({ senderAddress: testAddress1 }),
						createReceipt({ senderAddress: testAddress2 })
					]),
					createTransactionStatement(30, 120, [createReceipt({ senderAddress: testAddress2 })]),
					createTransactionStatement(40, 130, [])
				];
				const filters = { senderAddress: testAddress2 };

				// Act + Assert:
				return runTestAndVerifyIds(
					dbReceipts,
					collectionName,
					db => db.transactionStatements(filters, paginationOptions),
					[20, 30]
				);
			});

			it('targetAddress', () => {
				// Arrange:
				// Arrange:
				const dbReceipts = [
					createTransactionStatement(10, 100, [createReceipt({ targetAddress: testAddress1 })]),
					createTransactionStatement(20, 110, [
						createReceipt({ targetAddress: testAddress1 }),
						createReceipt({ targetAddress: testAddress2 })
					]),
					createTransactionStatement(30, 120, [createReceipt({ targetAddress: testAddress2 })]),
					createTransactionStatement(40, 130, [])
				];
				const filters = { targetAddress: testAddress2 };

				// Act + Assert:
				return runTestAndVerifyIds(
					dbReceipts,
					collectionName,
					db => db.transactionStatements(filters, paginationOptions),
					[20, 30]
				);
			});

			describe('artifactId', () => {
				// NamespaceIds examples: 941299B2B7E1291C, 85BBEA6CC462B244, CB9F84B545BA480C
				// MosaicIds examples: 1D9CDC7E218CA88D, 24F426B8D5493D4B, 49F6C0F0163730A9

				const namespaceId1 = uint64.fromHex('941299B2B7E1291C');
				const namespaceId2 = uint64.fromHex('85BBEA6CC462B244');
				const mosaicId1 = uint64.fromHex('1D9CDC7E218CA88D');
				const mosaicId2 = uint64.fromHex('24F426B8D5493D4B');

				it('namespaceId', () => {
					// Arrange:
					const dbReceipts = [
						createTransactionStatement(10, 100, [createReceipt({ namespaceId: namespaceId1, mosaicId: namespaceId2 })]),
						createTransactionStatement(20, 110, [
							createReceipt({ namespaceId: namespaceId1 }),
							createReceipt({ namespaceId: namespaceId2 })
						]),
						createTransactionStatement(30, 120, [createReceipt({ namespaceId: namespaceId2 })]),
						createTransactionStatement(40, 130, [])
					];
					const filters = { artifactId: namespaceId2 };

					// Act + Assert:
					return runTestAndVerifyIds(
						dbReceipts,
						collectionName,
						db => db.transactionStatements(filters, paginationOptions),
						[20, 30]
					);
				});

				it('mosaicId', () => {
					// Arrange:
					const dbReceipts = [
						createTransactionStatement(10, 100, [createReceipt({ namespaceId: mosaicId2, mosaicId: mosaicId1 })]),
						createTransactionStatement(20, 110, [
							createReceipt({ mosaicId: mosaicId1 }),
							createReceipt({ mosaicId: mosaicId2 })
						]),
						createTransactionStatement(30, 120, [createReceipt({ mosaicId: mosaicId2 })]),
						createTransactionStatement(40, 130, [])
					];
					const filters = { artifactId: mosaicId2 };

					// Act + Assert:
					return runTestAndVerifyIds(
						dbReceipts,
						collectionName,
						db => db.transactionStatements(filters, paginationOptions),
						[20, 30]
					);
				});
			});
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbReceipts = () => ([
				createTransactionStatement(10, 30),
				createTransactionStatement(20, 20),
				createTransactionStatement(30, 10)
			]);

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runReceiptsDbTest(
					dbReceipts(),
					collectionName,
					db => db.transactionStatements({}, options),
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
				return runReceiptsDbTest(
					dbReceipts(),
					collectionName,
					db => db.transactionStatements({}, options),
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
				return runReceiptsDbTest(
					dbReceipts(),
					collectionName,
					db => db.transactionStatements({}, options),
					() => {
						expect(queryPagedDocumentsSpy.calledOnce).to.equal(true);
						expect(Object.keys(queryPagedDocumentsSpy.firstCall.args[2])[0]).to.equal('_id');
						queryPagedDocumentsSpy.restore();
					}
				);
			});
		});

		describe('respects offset', () => {
			// Arrange:
			const dbReceipts = () => ([
				createTransactionStatement(10, 30),
				createTransactionStatement(20, 20),
				createTransactionStatement(30, 10)
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
				return runTestAndVerifyIds(dbReceipts(), collectionName, db => db.transactionStatements({}, options), [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbReceipts(), collectionName, db => db.transactionStatements({}, options), [10]);
			});
		});
	});

	describe('artifactStatements', () => {
		const artifactTypes = [
			{
				type: 'address',
				createResolutionStatement: (objectId, height) => ({
					_id: createObjectId(objectId),
					statement: {
						height: Long.fromNumber(height),
						unresolved: new Binary(Buffer.from(testAddress1)),
						resolutionEntries: []
					}
				})
			},
			{
				type: 'mosaic',
				createResolutionStatement: (objectId, height) => ({
					_id: createObjectId(objectId),
					statement: { height: Long.fromNumber(height), unresolved: Long.fromNumber(5432), resolutionEntries: [] }
				})
			}
		];

		artifactTypes.forEach(artifact => {
			const collectionName = `${artifact.type}ResolutionStatements`;

			describe(`${artifact.type}ResolutionStatements`, () => {
				it('returns expected structure', () => {
					// Arrange:
					const sampleResolutionStatement = artifact.createResolutionStatement(10, 100);
					const dbReceipts = [sampleResolutionStatement];

					// Act + Assert:
					return runReceiptsDbTest(
						dbReceipts,
						collectionName,
						db => db.artifactStatements(100, artifact.type, paginationOptions),
						page => {
							const expected_keys = ['id', 'statement'];
							expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
							expect(page.data[0].statement).to.deep.equal(sampleResolutionStatement.statement);
						}
					);
				});

				it('returns empty array for empty height', () => {
					// Arrange:
					const dbReceipts = [artifact.createResolutionStatement(10, 100)];

					// Act + Assert:
					return runTestAndVerifyIds(
						dbReceipts,
						collectionName,
						db => db.artifactStatements(200, artifact.type, paginationOptions),
						[]
					);
				});

				it('returns all the statements if no height is provided', () => {
					// Arrange:
					const dbReceipts = [
						artifact.createResolutionStatement(10, 100),
						artifact.createResolutionStatement(20, 110),
						artifact.createResolutionStatement(30, 120)
					];

					// Act + Assert:
					return runTestAndVerifyIds(
						dbReceipts,
						collectionName,
						db => db.artifactStatements(undefined, artifact.type, paginationOptions),
						[10, 20, 30]
					);
				});

				it('returns filtered statements by height', () => {
					// Arrange:
					const dbReceipts = [
						artifact.createResolutionStatement(10, 100),
						artifact.createResolutionStatement(20, 110),
						artifact.createResolutionStatement(30, 120)
					];

					// Act + Assert:
					return runTestAndVerifyIds(
						dbReceipts,
						collectionName,
						db => db.artifactStatements(110, artifact.type, paginationOptions),
						[20]
					);
				});

				describe('respects sort conditions', () => {
					// Arrange:
					const dbReceipts = () => ([
						artifact.createResolutionStatement(10, 30),
						artifact.createResolutionStatement(20, 20),
						artifact.createResolutionStatement(30, 10)
					]);

					it('direction ascending', () => {
						const options = {
							pageSize: 10,
							pageNumber: 1,
							sortField: 'id',
							sortDirection: 1
						};

						// Act + Assert:
						return runReceiptsDbTest(
							dbReceipts(),
							collectionName,
							db => db.artifactStatements(undefined, artifact.type, options),
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
						return runReceiptsDbTest(
							dbReceipts(),
							collectionName,
							db => db.artifactStatements(undefined, artifact.type, options),
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
						return runReceiptsDbTest(
							dbReceipts(),
							collectionName,
							db => db.artifactStatements(undefined, artifact.type, options),
							() => {
								expect(queryPagedDocumentsSpy.calledOnce).to.equal(true);
								expect(Object.keys(queryPagedDocumentsSpy.firstCall.args[2])[0]).to.equal('_id');
								queryPagedDocumentsSpy.restore();
							}
						);
					});
				});

				describe('respects offset', () => {
					// Arrange:
					const dbReceipts = () => ([
						artifact.createResolutionStatement(10, 30),
						artifact.createResolutionStatement(20, 20),
						artifact.createResolutionStatement(30, 10)
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
						return runTestAndVerifyIds(
							dbReceipts(),
							collectionName,
							db => db.artifactStatements(undefined, artifact.type, options),
							[30]
						);
					});

					it('lt', () => {
						options.sortDirection = -1;

						// Act + Assert:
						return runTestAndVerifyIds(
							dbReceipts(),
							collectionName,
							db => db.artifactStatements(undefined, artifact.type, options),
							[10]
						);
					});
				});
			});
		});
	});
});
