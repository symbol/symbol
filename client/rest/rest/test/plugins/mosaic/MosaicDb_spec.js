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

const test = require('./mosaicDbTestUtils');
const AccountType = require('../../../src/plugins/AccountType');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');

const { Binary, Long } = MongoDb;
const { address } = catapult.model;
const { convert } = catapult.utils;

describe('mosaic db', () => {
	describe('mosaics by ids', () => {
		const createMosaics = (numNamespaces, numMosaicsPerNamespace) => {
			const ownerPublicKey = test.random.publicKey();
			return test.db.createMosaics(ownerPublicKey, numNamespaces, numMosaicsPerNamespace);
		};

		it('returns empty array for unknown mosaic ids', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns single matching mosaic', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0]]),
				entities => { expect(entities).to.deep.equal([mosaics[10]]); }
			);
		});

		it('returns multiple matching mosaics', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10007, 0], [10003, 0]]),
				entities => { expect(entities).to.deep.equal([mosaics[10], mosaics[7], mosaics[3]]); }
			);
		});

		it('returns only known mosaics', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10021, 0], [10003, 0]]),
				entities => expect(entities).to.deep.equal([mosaics[10], mosaics[3]])
			);
		});
	});

	describe('mosaics by owners', () => {
		const testAddress = {
			one: address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV'),
			two: address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG'),
			three: address.stringToAddress('SAAM2O7SSJ2A7AU3DZJMSTTRFZT5TFDPQ3ZIIJX7'),
			four: address.stringToAddress('SAMZMPX33DFIIVOCNJYMF5KJTGLAEVNKHHFROLXD')
		};
		const testPublicKey = {
			one: convert.hexToUint8('7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7'),
			two: convert.hexToUint8('75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C'),
			three: convert.hexToUint8('5AD98F5C983599634C9C9B1ECAA2B2B2B1AAB3F741D4C256CEE4D866EA5A92D1'),
			four: convert.hexToUint8('A966DA3D73BA18B55C83E64CE4C38ACB29E38CF38B4E6C1789E7C1B254E0CB89')
		};

		const createMosaic = (mosaicId, ownerPublicKey, ownerAddress) => ({
			_id: Long.fromNumber(mosaicId),
			mosaic: {
				id: Long.fromNumber(mosaicId),
				ownerPublicKey: new Binary(ownerPublicKey),
				ownerAddress: new Binary(ownerAddress)
			}
		});

		// Arrange:
		const mosaic = createMosaic(1000, Buffer.from(testPublicKey.one), Buffer.from(testAddress.one));
		const mosaic2 = createMosaic(2000, Buffer.from(testPublicKey.two), Buffer.from(testAddress.two));
		const mosaic3 = createMosaic(3000, Buffer.from(testPublicKey.three), Buffer.from(testAddress.three));

		it('returns empty array for unknown address', () =>
			// Act + Assert:
			test.db.runDbTest(
				[mosaic, mosaic2],
				db => db.mosaicsByOwners(AccountType.address, [testAddress.three, testAddress.four]),
				entities => { expect(entities).to.deep.equal([]); }
			));

		it('returns empty array for unknown public key', () =>
			// Act + Assert:
			test.db.runDbTest(
				[mosaic, mosaic2],
				db => db.mosaicsByOwners(AccountType.publicKey, [testPublicKey.three, testPublicKey.four]),
				entities => { expect(entities).to.deep.equal([]); }
			));

		it('returns single matching mosaic by address', () =>
			// Act + Assert:
			test.db.runDbTest(
				[mosaic, mosaic2, mosaic3],
				db => db.mosaicsByOwners(AccountType.address, [testAddress.two]),
				entities => { expect(entities).to.deep.equal([mosaic2.mosaic]); }
			));

		it('returns single matching mosaic by public key', () =>
			// Act + Assert:
			test.db.runDbTest(
				[mosaic, mosaic2, mosaic3],
				db => db.mosaicsByOwners(AccountType.publicKey, [testPublicKey.two]),
				entities => { expect(entities).to.deep.equal([mosaic2.mosaic]); }
			));

		it('returns multiple matching mosaic by address', () =>
			// Act + Assert:
			test.db.runDbTest(
				[mosaic, mosaic2, mosaic3],
				db => db.mosaicsByOwners(AccountType.address, [testAddress.one, testAddress.two]),
				entities => { expect(entities).to.deep.equal([mosaic.mosaic, mosaic2.mosaic]); }
			));

		it('returns multiple matching mosaic by public key', () =>
			// Act + Assert:
			test.db.runDbTest(
				[mosaic, mosaic2, mosaic3],
				db => db.mosaicsByOwners(AccountType.publicKey, [testPublicKey.one, testPublicKey.two]),
				entities => { expect(entities).to.deep.equal([mosaic.mosaic, mosaic2.mosaic]); }
			));
	});
});
