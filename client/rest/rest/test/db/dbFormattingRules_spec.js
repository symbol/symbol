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

const formattingRules = require('../../src/db/dbFormattingRules');
const { convertToLong } = require('../../src/db/dbUtils');
const test = require('../testUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const { Binary } = require('mongodb');

const { ModelType } = catapult.model;

describe('db formatting rules', () => {
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
		const object = test.factory.createBinary(Buffer.from('FEDCBA9876543210', 'hex'));

		// Act:
		const result = formattingRules[ModelType.binary](object);

		// Assert:
		expect(result).to.equal('FEDCBA9876543210');
	});

	it('can format javascript buffer as binary type', () => {
		// Arrange:
		const object = Buffer.from('FEDCBA9876543210', 'hex');

		// Act:
		const result = formattingRules[ModelType.binary](object);

		// Assert:
		expect(result).to.equal('FEDCBA9876543210');
	});

	it('can format object id type', () => {
		// Arrange:
		const object = test.factory.createObjectIdFromHexString('3AEDCBA9876F94725732547F');

		// Act:
		const result = formattingRules[ModelType.objectId](object);

		// Assert:
		expect(result).to.equal('3AEDCBA9876F94725732547F');
	});

	it('can format status code type', () => {
		// Arrange: notice that codes are signed in db
		[0x80530001, -2142044159].forEach(code => {
			// Act:
			const result = formattingRules[ModelType.statusCode](code);

			// Assert:
			expect(result, `${code} code`).to.equal('Failure_Signature_Not_Verifiable');
		});
	});

	it('can format string type', () => {
		// Arrange:
		const object = test.factory.createBinary(Buffer.from('6361746170756C74', 'hex'));

		// Act:
		const result = formattingRules[ModelType.string](object);

		// Assert:
		expect(result).to.equal('catapult');
	});

	describe('can format uint8 type', () => {
		const testCases = [
			{ name: 'value 0', value: 0, formated: 0 },
			{ name: 'value 128', value: 128, formated: 128 },
			{ name: 'value 255 (max)', value: 255, formated: 255 }
		];

		testCases.forEach(testCase => {
			it(testCase.name, () => {
				// Arrange + Act:
				const result = formattingRules[ModelType.uint8](testCase.value);

				// Assert:
				expect(result).to.equal(testCase.formated);
			});
		});
	});

	describe('can format uint16 type', () => {
		const testCases = [
			{ name: 'value 0', value: 0, formated: 0 },
			{ name: 'value 17434', value: 17434, formated: 17434 },
			{ name: 'value 32768', value: 32768, formated: 32768 },
			{ name: 'value 65535 (max)', value: 65535, formated: 65535 }
		];

		testCases.forEach(testCase => {
			it(testCase.name, () => {
				// Arrange + Act:
				const result = formattingRules[ModelType.uint16](testCase.value);

				// Assert:
				expect(result).to.equal(testCase.formated);
			});
		});

		it('can format uint16 type from Binary', () => {
			// Arrange:
			const buffer = Buffer.alloc(2, 0);
			buffer.writeUInt16LE(17434);
			const object = new Binary(buffer);

			// Act:
			const result = formattingRules[ModelType.uint16](object);

			// Assert:
			expect(result).to.deep.equal(17434);
		});
	});

	describe('can format uint32 type', () => {
		const testCases = [
			{ name: 'value 0', value: 0, formated: 0 },
			{ name: 'value 2147483647', value: 2147483647, formated: 2147483647 },
			{ name: 'value -2147483648', value: -2147483648, formated: 2147483648 },
			{ name: 'value 4294967295 (max)', value: -1, formated: 4294967295 }
		];

		testCases.forEach(testCase => {
			it(testCase.name, () => {
				// Arrange + Act:
				const result = formattingRules[ModelType.uint32](testCase.value);

				// Assert:
				expect(result).to.equal(testCase.formated);
			});
		});
	});

	describe('can format uint64 type', () => {
		it('can format uint64 type from Long', () => {
			// Arrange:
			const object = convertToLong([1, 2]);

			// Act:
			const result = formattingRules[ModelType.uint64](object);

			// Assert:
			expect(result).to.equal('8589934593');
		});

		it('can format uint64HexIdentifier type from Long', () => {
			// Arrange:
			const object = convertToLong([1, 2]);

			// Act:
			const result = formattingRules[ModelType.uint64HexIdentifier](object);

			// Assert:
			expect(result).to.equal('0000000200000001');
		});

		it('can format uint64HexIdentifier type from Binary', () => {
			// Arrange:
			const buffer = Buffer.alloc(8, 0);
			buffer.writeUInt32LE(0x00ABCDEF, 0);
			buffer.writeUInt32LE(0x000FDFFF, 4);
			const object = new Binary(buffer);

			// Act:
			const result = formattingRules[ModelType.uint64HexIdentifier](object);

			// Assert:
			expect(result).to.equal('000FDFFF00ABCDEF');
		});
	});

	describe('can format int type', () => {
		const testCases = [
			{ name: 'value 0', value: 0, formated: 0 },
			{ name: 'value 255', value: 255, formated: 255 },
			{ name: 'value 65535', value: 65535, formated: 65535 },
			{ name: 'value -1', value: -1, formated: -1 },
			{ name: 'value -2147483648 (min)', value: -2147483648, formated: -2147483648 },
			{ name: 'value 2147483647 (max)', value: 2147483647, formated: 2147483647 }
		];

		testCases.forEach(testCase => {
			it(testCase.name, () => {
				// Arrange + Act:
				const result = formattingRules[ModelType.int](testCase.value);

				// Assert:
				expect(result).to.equal(testCase.formated);
			});
		});
	});

	describe('can format boolean type', () => {
		const testCases = [
			{ name: 'boolean true', value: true, formated: true },
			{ name: 'boolean false', value: false, formated: false }
		];

		testCases.forEach(testCase => {
			it(testCase.name, () => {
				// Arrange + Act:
				const result = formattingRules[ModelType.boolean](testCase.value);

				// Assert:
				expect(result).to.equal(testCase.formated);
			});
		});
	});
});
