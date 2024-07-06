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

import {
	bufferToUnresolvedAddress, buildOffsetCondition, convertToLong, longToUint64, uniqueLongList
} from '../../src/db/dbUtils.js';
import { expect } from 'chai';
import MongoDb from 'mongodb';

const { ObjectId } = MongoDb;

describe('db utils', () => {
	describe('convertToLong', () => {
		it('can convert from integer to long', () => {
			// Act + Assert:
			expect(convertToLong(123)).to.deep.equal(new MongoDb.Long(123, 0));
		});

		it('can convert from bigint to long', () => {
			// Act + Assert:
			expect(convertToLong(0x1234567890ABCDEFn)).to.deep.equal(new MongoDb.Long(0x90ABCDEF, 0x12345678));
		});

		it('can convert from negative one to long', () => {
			// Act + Assert:
			expect(convertToLong(-1)).to.deep.equal(MongoDb.Long.NEG_ONE);
		});

		it('can convert from one to long', () => {
			// Act + Assert:
			expect(convertToLong(1)).to.deep.equal(MongoDb.Long.ONE);
			expect(convertToLong(1n)).to.deep.equal(MongoDb.Long.ONE);
		});

		it('can convert from zero to long', () => {
			// Act + Assert:
			expect(convertToLong(0)).to.deep.equal(MongoDb.Long.ZERO);
			expect(convertToLong(0n)).to.deep.equal(MongoDb.Long.ZERO);
		});

		it('returns same value if value is already long', () => {
			// Arrange
			const longValue = MongoDb.Long.fromNumber(12345);

			// Act + Assert:
			expect(convertToLong(longValue)).to.deep.equal(longValue);
		});

		it('throws error if value not integer and not array', () => {
			// Act + Assert:
			expect(() => convertToLong('abc')).to.throw('abc has an invalid format: not integer or bigint');
			expect(() => convertToLong([123, 456])).to.throw('123,456 has an invalid format: not integer or bigint');
		});
	});

	describe('longToUint64', () => {
		it('can convert from long to bigint', () => {
			// Act + Assert:
			expect(longToUint64(new MongoDb.Long(0x90ABCDEF, 0x12345678))).to.equal(0x1234567890ABCDEFn);
		});

		it('can convert from long (negative) to bigint', () => {
			// Act + Assert:
			expect(longToUint64(new MongoDb.Long(0xF1193A4A, 0xCE011F45))).to.equal(0xCE011F45F1193A4An);
		});

		it('can convert from one, long value, to uint64', () => {
			// Act + Assert:
			expect(longToUint64(MongoDb.Long.ONE)).to.equal(1n);
		});

		it('can convert from zero, long value, to uint64', () => {
			// Act + Assert:
			expect(longToUint64(MongoDb.Long.ZERO)).to.equal(0n);
		});

		it('throws error if value not long', () => {
			// Act + Assert:
			expect(() => longToUint64('abc')).to.throw('abc has an invalid format: not long');
		});
	});

	describe('uniqueLongList', () => {
		it('unique list empty', () => {
			// Act + Assert
			expect(uniqueLongList([])).to.deep.equal([]);
		});

		it('unique list not duplicated', () => {
			// Act + Assert
			expect(uniqueLongList([convertToLong(1), convertToLong(2), convertToLong(3)]))
				.to.deep.equal([convertToLong(1), convertToLong(2), convertToLong(3)]);
		});

		it('unique list duplicated', () => {
			// Act + Assert
			expect(uniqueLongList([convertToLong(3), convertToLong(1), convertToLong(3)]))
				.to.deep.equal([convertToLong(3), convertToLong(1)]);
		});
	});

	describe('buildOffsetCondition', () => {
		it('undefined offset', () => {
			// Arrange
			const options = { offset: undefined };
			const sortFieldDbRelation = { id: '_id' };

			// Act + Assert
			expect(buildOffsetCondition(options, sortFieldDbRelation)).to.equal(undefined);
		});

		it('can create object id offset condition', () => {
			// Arrange
			const options = {
				offset: '112233445566778899AABBCC',
				offsetType: 'objectId',
				sortField: 'id',
				sortDirection: 'desc'
			};
			const sortFieldDbRelation = { id: '_id' };

			// Act + Assert
			expect(buildOffsetCondition(options, sortFieldDbRelation)).to.deep.equal({
				_id: { $lt: new ObjectId('112233445566778899AABBCC') }
			});
		});

		it('can create uint64 offset condition', () => {
			// Arrange
			const options = {
				offset: 0xAABBCCDDn,
				offsetType: 'uint64',
				sortField: 'height',
				sortDirection: 'desc'
			};
			const sortFieldDbRelation = { height: 'height' };

			// Act + Assert
			expect(buildOffsetCondition(options, sortFieldDbRelation)).to.deep.equal({
				height: { $lt: convertToLong(0xAABBCCDDn) }
			});
		});

		it('can create uint64Hex offset condition', () => {
			// Arrange
			const options = {
				offset: 0xAABBCCDDn,
				offsetType: 'uint64Hex',
				sortField: 'id',
				sortDirection: 'desc'
			};
			const sortFieldDbRelation = { id: '_id' };

			// Act + Assert
			expect(buildOffsetCondition(options, sortFieldDbRelation)).to.deep.equal({
				_id: { $lt: convertToLong(0xAABBCCDDn) }
			});
		});
	});

	describe('bufferToUnresolvedAddress', () => {
		it('can convert from Buffer to encoded address', () => {
			// Arrange
			const object = Buffer.from('98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56', 'hex');

			// Act:
			const result = bufferToUnresolvedAddress(object, true);

			// Assert:
			expect(result).to.equal('TDQNCOHK6KWDILABL7YLMMPMGYRORL72AS74YVQ');
		});

		it('can convert from Buffer to decoded address', () => {
			// Arrange
			const object = Buffer.from('98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56', 'hex');

			// Act:
			const result = bufferToUnresolvedAddress(object);

			// Assert:
			expect(result).to.equal('98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56');
		});

		it('can convert from undefined to undefined address', () => {
			// Arrange
			const object = undefined;

			// Act:
			const result = bufferToUnresolvedAddress(object);

			// Assert:
			expect(result).to.equal(undefined);
		});

		it('cannot convert from invalid data type', () => {
			// Arrange
			const object = '99CAAB0FD01CCF25BA000000000000000000000000000000';

			// act + Assert:
			expect(() => bufferToUnresolvedAddress(object)).to.throw('Cannot convert binary address, unknown String type');
		});
	});
});
