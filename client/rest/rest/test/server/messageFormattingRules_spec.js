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
const formattingRules = require('../../src/server/messageFormattingRules');
const test = require('../testUtils');

const { ModelType } = catapult.model;

describe('message formatting rules', () => {
	it('can format none type', () => {
		// Arrange:
		const object = { foo: 8 };

		// Act:
		const result = formattingRules[ModelType.none](object);

		// Assert:
		expect(result).to.deep.equal({ foo: 8 });
	});

	it('can format binary type', () => {
		// Arrange:
		const object = Buffer.from('FEDCBA9876543210', 'hex');

		// Act:
		const result = formattingRules[ModelType.binary](object);

		// Assert:
		expect(result).to.equal('FEDCBA9876543210');
	});

	it('can format uint64 type', () => {
		// Arrange:
		const object = [1, 2];

		// Act:
		const result = formattingRules[ModelType.uint64](object);

		// Assert:
		expect(result).to.deep.equal([1, 2]);
	});

	it('cannot format object id type', () => {
		// Assert: objectId should never be written into messages, so it should be dropped
		expect(Object.keys(formattingRules)).to.not.contain.key(ModelType.objectId);
	});

	it('can format string type', () => {
		// Arrange:
		const object = test.factory.createBinary(Buffer.from('6361746170756C74', 'hex'));

		// Act:
		const result = formattingRules[ModelType.string](object);

		// Assert:
		expect(result).to.equal('catapult');
	});

	it('can format status code type', () => {
		// Arrange:
		const code = 0x80530008;

		// Act:
		const result = formattingRules[ModelType.statusCode](code);

		// Assert:
		expect(result).to.equal('Failure_Signature_Not_Verifiable');
	});
});
