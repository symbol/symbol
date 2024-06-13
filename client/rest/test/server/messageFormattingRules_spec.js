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

import catapult from '../../src/catapult-sdk/index.js';
import formattingRules from '../../src/server/messageFormattingRules.js';
import test from '../testUtils.js';
import { expect } from 'chai';

const { ModelType } = catapult.model;

describe('message formatting rules', () => {
	const assertFormatting = (modelType, input, expectedResult) => {
		// Act:
		const result = formattingRules[modelType](input);

		// Assert:
		expect(result).to.deep.equal(expectedResult);
	};

	it('can format none type', () => {
		assertFormatting(ModelType.none, { foo: 8 }, { foo: 8 });
	});

	it('can format binary type', () => {
		assertFormatting(ModelType.binary, Buffer.from('FEDCBA9876543210', 'hex'), 'FEDCBA9876543210');
	});

	it('can format binary type (string)', () => {
		assertFormatting(ModelType.binary, 'FEDCBA9876543210', 'FEDCBA9876543210');
	});

	it('cannot format object id type', () => {
		// Assert: objectId should never be written into messages, so it should be dropped
		expect(formattingRules).to.not.contain.key(ModelType.objectId);
	});

	it('can format status code type', () => {
		assertFormatting(ModelType.statusCode, 0x80530001, 'Failure_Signature_Not_Verifiable');
	});

	it('can format string type', () => {
		assertFormatting(ModelType.string, test.factory.createBinary(Buffer.from('6361746170756C74', 'hex')), 'catapult');
	});

	it('can format uint8 type', () => {
		assertFormatting(ModelType.uint8, 56, 56);
	});

	it('can format uint16 type', () => {
		assertFormatting(ModelType.uint16, 1234, 1234);
	});

	it('can format uint32 type', () => {
		assertFormatting(ModelType.uint32, 12345678, 12345678);
	});

	it('can format uint64 type', () => {
		assertFormatting(ModelType.uint64, 8589934602n, '8589934602');
	});

	it('can format uint64 type (string)', () => {
		assertFormatting(ModelType.uint64, '8589934602', '8589934602');
	});

	it('can format uint64HexIdentifier type', () => {
		assertFormatting(ModelType.uint64HexIdentifier, 8589934602n, '000000020000000A');
	});

	it('can format uint64HexIdentifier type (string)', () => {
		assertFormatting(ModelType.uint64HexIdentifier, '8589934602', '000000020000000A');
	});

	it('can format int type', () => {
		assertFormatting(ModelType.int, 12345678, 12345678);
	});

	it('can format boolean type', () => {
		assertFormatting(ModelType.boolean, true, true);
	});

	it('can format encodedAddress type', () => {
		assertFormatting(
			ModelType.encodedAddress,
			test.factory.createBinary(Buffer.from('98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56', 'hex')),
			'98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56'
		);
	});

	it('can format encodedAddress type (string)', () => {
		assertFormatting(
			ModelType.encodedAddress,
			'98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56',
			'98E0D138EAF2AC342C015FF0B631EC3622E8AFFA04BFCC56'
		);
	});
});
