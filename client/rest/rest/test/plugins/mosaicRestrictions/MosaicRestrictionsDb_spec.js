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

const MosaicRestrictionsDbTestUtils = require('./MosaicRestrictionsDbTestUtils');
const { expect } = require('chai');
const catapult = require('catapult-sdk');

const { address, mosaicRestriction } = catapult.model;

describe('mosaic restrictions db', () => {
	const dbUtils = MosaicRestrictionsDbTestUtils;
	const testAddress = {
		one: address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV'),
		two: address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG'),
		three: address.stringToAddress('SAAM2O7SSJ2A7AU3DZJMSTTRFZT5TFDPQ3ZIIJX7'),
		four: address.stringToAddress('SAMZMPX33DFIIVOCNJYMF5KJTGLAEVNKHHFROLXD')
	};

	describe('mosaic restrictions by mosaic ids', () => {
		it('returns empty for no restrictions with mosaicId', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction([0xAAAAAAAA, 0xAAAAAAAA]);
			const restriction2 = dbUtils.createAddressMosaicRestriction([0xAAAAAAAA, 0xAAAAAAAA], testAddress.one);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2],
				db => db.mosaicRestrictionsByMosaicIds([[0xBBBBBBBB, 0xBBBBBBBB]], mosaicRestriction.restrictionType.global),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns correct restrictions for one mosaic id', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction([0x32ABFE33, 0x876EEC50]);
			const restriction2 = dbUtils.createGlobalMosaicRestriction([0xAAAAAAAA, 0xBBBBBBBB]);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2],
				db => db.mosaicRestrictionsByMosaicIds([[0x32ABFE33, 0x876EEC50]], mosaicRestriction.restrictionType.global),
				entities => { expect(entities).to.deep.equal([dbUtils.sanitizeId(restriction1)]); }
			);
		});

		it('returns correct restrictions for several mosaic ids', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction([0xFD7657CC, 0x5B64C36F]);
			const restriction2 = dbUtils.createGlobalMosaicRestriction([0xAAAAAAAA, 0xBBBBBBBB]);
			const restriction3 = dbUtils.createGlobalMosaicRestriction([0x6767FF54, 0x12AFF673]);
			const restriction4 = dbUtils.createAddressMosaicRestriction([0xFD7657CC, 0x5B64C36F], testAddress.one);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2, restriction3, restriction4],
				db => db.mosaicRestrictionsByMosaicIds(
					[[0xFD7657CC, 0x5B64C36F], [0x6767FF54, 0x12AFF673]],
					mosaicRestriction.restrictionType.global
				),
				entities => { expect(entities).to.deep.equal([dbUtils.sanitizeId(restriction1), dbUtils.sanitizeId(restriction3)]); }
			);
		});

		it('correctly filters for restriction type', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction([0xAAAAAAAA, 0xBBBBBBBB]);
			const restriction2 = dbUtils.createAddressMosaicRestriction([0xAAAAAAAA, 0xBBBBBBBB], testAddress.one);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2],
				db => db.mosaicRestrictionsByMosaicIds([[0xAAAAAAAA, 0xBBBBBBBB]], mosaicRestriction.restrictionType.address),
				entities => { expect(entities).to.deep.equal([dbUtils.sanitizeId(restriction2)]); }
			);
		});
	});

	describe('mosaic address restrictions', () => {
		const mosaicId = [0xAAAAAAAA, 0xBBBBBBBB];

		it('returns empty for no restrictions with target address', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction(mosaicId);
			const restriction2 = dbUtils.createAddressMosaicRestriction(mosaicId, testAddress.one);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2],
				db => db.mosaicAddressRestrictions(mosaicId, [testAddress.two]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns correct address restrictions for one target address', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction(mosaicId);
			const restriction2 = dbUtils.createAddressMosaicRestriction(mosaicId, testAddress.one);
			const restriction3 = dbUtils.createAddressMosaicRestriction(mosaicId, testAddress.two);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2, restriction3],
				db => db.mosaicAddressRestrictions(mosaicId, [testAddress.one]),
				entities => { expect(entities).to.deep.equal([dbUtils.sanitizeId(restriction2)]); }
			);
		});

		it('returns correct address restrictions for several target addresses', () => {
			// Arrange:
			const restriction1 = dbUtils.createGlobalMosaicRestriction(mosaicId);
			const restriction2 = dbUtils.createAddressMosaicRestriction(mosaicId, testAddress.one);
			const restriction3 = dbUtils.createAddressMosaicRestriction(mosaicId, testAddress.three);
			const restriction4 = dbUtils.createAddressMosaicRestriction(mosaicId, testAddress.four);

			// Act + Assert:
			return dbUtils.runDbTest(
				[restriction1, restriction2, restriction3, restriction4],
				db => db.mosaicAddressRestrictions(mosaicId, [testAddress.one, testAddress.four]),
				entities => { expect(entities).to.deep.equal([dbUtils.sanitizeId(restriction2), dbUtils.sanitizeId(restriction4)]); }
			);
		});
	});
});
