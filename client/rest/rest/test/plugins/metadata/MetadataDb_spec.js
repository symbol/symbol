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

const MetadataDb = require('../../../src/plugins/metadata/MetadataDb');
const test = require('../../db/utils/dbTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');

const { address } = catapult.model;
const { Binary, ObjectId, Long } = MongoDb;

describe('metadata db', () => {
	const testAddress = {
		one: 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV',
		two: 'SAMZMPX33DFIIVOCNJYMF5KJTGLAEVNKHHFROLXD'
	};

	const senderTestAddress = {
		one: 'SCFZFP7N5C3P6EHP5D2UJ7GQD7Q7ZIENV4NZ6ELN',
		two: 'SAAM2O7SSJ2A7AU3DZJMSTTRFZT5TFDPQ3ZIIJX7'
	};

	const scopedMetadataKey = {
		one: [0xBAF454E1, 0xCC7676F3],
		two: [0xC345AA21, 0xB4512EE0]
	};

	const createObjectId = id => new ObjectId(`${'00'.repeat(12)}${id}`.slice(-24));

	const createMetadata = (id, metadataType, accountAddress, scopedKey, senderAddress, value) => ({
		_id: -1 === id ? createObjectId(Math.floor(Math.random() * 100000)) : id,
		metadataEntry: {
			compositeHash: {},
			sourceAddress: undefined !== senderAddress ? new Binary(Buffer.from(address.stringToAddress(senderAddress))) : '',
			targetAddress: undefined !== accountAddress ? new Binary(Buffer.from(address.stringToAddress(accountAddress))) : '',
			scopedMetadataKey: undefined !== scopedKey ? new Long(scopedKey[0], scopedKey[1]) : '',
			targetId: 0,
			metadataType: undefined !== metadataType ? metadataType : '',
			valueSize: undefined === value ? 0 : value.length,
			value: undefined !== value ? new Binary(value) : ''
		}
	});

	const targetFilter = { 'metadataEntry.targetAddress': Buffer.from(address.stringToAddress(testAddress.one)) };

	describe('get metadata with pagination', () => {
		it('can get metadata with no pagination', () => {
			// Arrange:
			const searchedId = 100;
			const searchedMetadataType = 1;

			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(searchedId, searchedMetadataType, testAddress.one));
			metadataDbEntities.push(createMetadata(searchedId + 10, searchedMetadataType + 1, testAddress.one));
			metadataDbEntities.push(createMetadata(searchedId + 20, searchedMetadataType, testAddress.two));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataWithPagination(searchedMetadataType, targetFilter),
				entities => {
					expect(entities.length).to.equal(1);
					expect(entities[0].id).to.equal(searchedId);
				}
			);
		});

		it('can get metadata with pagination', () => {
			// Arrange:
			const searchedMetadataType = 1;
			const startingPageId = 10;
			const pageSize = 20;

			const metadataDbEntities = [];
			for (let index = 1; startingPageId + pageSize + 10 >= index; ++index)
				metadataDbEntities.push(createMetadata(createObjectId(index), searchedMetadataType, testAddress.one));

			const expectedPagedIds = [];
			for (let i = startingPageId + 1; startingPageId + pageSize >= i; ++i)
				expectedPagedIds.push(createObjectId(i));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataWithPagination(searchedMetadataType, targetFilter, createObjectId(startingPageId), pageSize, 1),
				entities => {
					const pagedIds = entities.map(e => e.id);
					expect(pagedIds).to.deep.equal(expectedPagedIds);
				}
			);
		});

		it('can get metadata with pagination, no pageId first time', () => {
			// Arrange:
			const searchedMetadataType = 1;
			const pageSize = 20;

			const metadataDbEntities = [];
			for (let index = 1; pageSize + 10 >= index; ++index)
				metadataDbEntities.push(createMetadata(createObjectId(index), searchedMetadataType, testAddress.one));

			const expectedPagedIds = [];
			for (let i = 1; pageSize >= i; ++i)
				expectedPagedIds.push(createObjectId(i));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataWithPagination(searchedMetadataType, targetFilter, undefined, pageSize, 1),
				entities => {
					const pagedIds = entities.map(e => e.id);
					expect(pagedIds).to.deep.equal(expectedPagedIds);
				}
			);
		});

		it('returns empty when filter resolves to false', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1));
			metadataDbEntities.push(createMetadata(-1));
			metadataDbEntities.push(createMetadata(-1));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataWithPagination(1, { 'metadataEntry.nonExistingField': '' }),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});
	});

	describe('get metadata by key', () => {
		it('gets metadata by key', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.two));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.two, senderTestAddress.one));
			metadataDbEntities.push(createMetadata(-1, 2, testAddress.one, scopedMetadataKey.one, senderTestAddress.two));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.two, scopedMetadataKey.one, senderTestAddress.one));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKey(1, targetFilter, scopedMetadataKey.one),
				entities => {
					const expectedSenderAddresses = [
						new Binary(Buffer.from(address.stringToAddress(senderTestAddress.one))),
						new Binary(Buffer.from(address.stringToAddress(senderTestAddress.two)))
					];
					const senderAddresses = entities.map(e => e.metadataEntry.sourceAddress);
					expect(entities.length).to.equal(2);
					expect(senderAddresses).to.deep.equal(expectedSenderAddresses);
				}
			);
		});

		it('returns empty when filter resolves to false', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKey(1, { 'metadataEntry.nonExistingField': '' }, scopedMetadataKey.one),
				entities => {
					expect(entities).to.deep.equal([]);
				}
			);
		});

		it('returns empty when there are no metadata with supplied key', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKey(1, targetFilter, scopedMetadataKey.two),
				entities => {
					expect(entities).to.deep.equal([]);
				}
			);
		});
	});

	describe('get metadata by key and sender', () => {
		it('gets metadata by key and sender', () => {
			// Arrange:
			const metadataDbEntities = [];
			const appleId = createObjectId(126);
			metadataDbEntities.push(createMetadata(appleId, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one, 'apple'));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.two, 'banana'));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.two, senderTestAddress.one, 'cherry'));
			metadataDbEntities.push(createMetadata(-1, 2, testAddress.one, scopedMetadataKey.one, senderTestAddress.two, 'dates'));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.two, scopedMetadataKey.one, senderTestAddress.one, 'entawak'));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKeyAndSender(
					1,
					targetFilter,
					scopedMetadataKey.one,
					Buffer.from(address.stringToAddress(senderTestAddress.one))
				),
				metadataEntry => {
					expect(metadataEntry).to.deep.equal({
						id: appleId,
						metadataEntry: {
							compositeHash: {},
							metadataType: 1,
							scopedMetadataKey: new Long(scopedMetadataKey.one[0], scopedMetadataKey.one[1]),
							sourceAddress: new Binary(Buffer.from(address.stringToAddress(senderTestAddress.one))),
							targetId: 0,
							targetAddress: new Binary(Buffer.from(address.stringToAddress(testAddress.one))),
							value: new Binary('apple'),
							valueSize: 5
						}
					});
				}
			);
		});

		it('returns undefined when filter resolves to false', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKeyAndSender(
					1,
					{ 'metadataEntry.nonExistingField': '' },
					scopedMetadataKey.one,
					Buffer.from(address.stringToAddress(senderTestAddress.one))
				),
				entities => {
					expect(entities).to.equal(undefined);
				}
			);
		});

		it('returns undefined when there are no metadata with supplied key', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one, 'apple'));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one, 'banana'));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKeyAndSender(
					1,
					targetFilter,
					scopedMetadataKey.two,
					Buffer.from(address.stringToAddress(senderTestAddress.one))
				),
				metadataEntry => {
					expect(metadataEntry).to.equal(undefined);
				}
			);
		});

		it('returns undefined when there are no metadata with supplied sender', () => {
			// Arrange:
			const metadataDbEntities = [];
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one, 'apple'));
			metadataDbEntities.push(createMetadata(-1, 1, testAddress.one, scopedMetadataKey.one, senderTestAddress.one, 'banana'));

			// Act + Assert:
			return test.db.runDbTest(
				metadataDbEntities,
				'metadata',
				db => new MetadataDb(db),
				db => db.getMetadataByKeyAndSender(
					1,
					targetFilter,
					scopedMetadataKey.one,
					Buffer.from(address.stringToAddress(senderTestAddress.two))
				),
				metadataEntry => {
					expect(metadataEntry).to.equal(undefined);
				}
			);
		});
	});
});
