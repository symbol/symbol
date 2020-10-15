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

const SchemaType = require('../../src/utils/SchemaType');
const schemaFormatter = require('../../src/utils/schemaFormatter');
const { expect } = require('chai');

describe('schema formatter', () => {
	describe('basic triggering', () => {
		const assertFormattingRuleIsTriggered = (propertyType, propertySchema) => {
			// Arrange: set up a formatting rule for the foo property type
			const sample = { foo: [1, 2] };
			const schema = { foo: propertySchema };
			const formattingRules = { [propertyType]: value => value[0] };

			// Act:
			const format = schemaFormatter.format(sample, schema, {}, formattingRules);

			// Assert:
			expect(format).to.deep.equal({ foo: 1 });
		};

		it('can format property with custom type (number) in schema with matching formatting rule', () => {
			// Assert:
			assertFormattingRuleIsTriggered(111, 111);
		});

		it('can format property with custom type (object) in schema with matching formatting rule', () => {
			// Assert:
			assertFormattingRuleIsTriggered(111, { type: 111 });
		});

		it('can format property with with type of value 0 (ModelType.none)', () => {
			// Assert:
			assertFormattingRuleIsTriggered(0, 0);
		});

		it('drops property if not defined in the schema', () => {
			// Arrange: set up a formatting rule for the foo property type
			const sample = { foo: [1, 2], bar: 3 };
			const schema = { foo: { type: 111 } };
			const formattingRules = { 111: value => value[0] };

			// Act:
			const format = schemaFormatter.format(sample, schema, {}, formattingRules);

			// Assert:
			expect(format).to.deep.equal({ foo: 1 });
		});

		it('can rename formatted property', () => {
			// Arrange: set up a formatting rule for the foo property type
			const sample = { foo: [1, 2] };
			const schema = { foo: { type: 112, resultKey: 'bar' } };
			const formattingRules = { 112: value => value[0] };

			// Act:
			const format = schemaFormatter.format(sample, schema, {}, formattingRules);

			// Assert:
			expect(format).to.deep.equal({ bar: 1 });
		});
	});

	describe('basic not triggering', () => {
		const assertFormattingRuleIsNotTriggered = (propertyType, propertySchema) => {
			// Arrange: set up a formatting rule for the foo property type
			const sample = { foo: [1, 2] };
			const schema = { foo: propertySchema };
			const formattingRules = { [propertyType]: value => value[0] };

			// Act:
			const format = schemaFormatter.format(sample, schema, {}, formattingRules);

			// Assert:
			expect(format).to.deep.equal({});
		};

		it('ignores property with custom type (number) in schema without matching formatting rule', () => {
			// Assert:
			assertFormattingRuleIsNotTriggered(SchemaType.none, 111);
		});

		it('ignores property with custom type (object) in schema without matching formatting rule', () => {
			// Assert:
			assertFormattingRuleIsNotTriggered(SchemaType.none, { type: 111 });
		});

		it('ignores property not in schema without default formatting rule', () => {
			// Assert:
			assertFormattingRuleIsNotTriggered(111, undefined);
		});
	});

	describe('nested formatting', () => {
		describe('object', () => {
			it('can format sub object', () => {
				// Arrange:
				const sample = { sub: { foo: 7 } };
				const schema = { sub: { type: SchemaType.object, schemaName: 'abc' } };
				const schemaDictionary = { abc: { foo: 11 } };
				const formattingRules = { 11: value => 2 * value };

				// Act:
				const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

				// Assert:
				expect(format).to.deep.equal({ sub: { foo: 14 } });
			});

			it('can format nested sub object (outer object same type)', () => {
				// Arrange:
				const subObjectPropertySchema = { type: SchemaType.object, schemaName: 'abc' };
				const sample = { sub: { foo: 7, sub2: { foo: 9 } } };
				const schema = { sub: subObjectPropertySchema };
				const schemaDictionary = { abc: { foo: 11, sub2: subObjectPropertySchema } };
				const formattingRules = { 11: value => 2 * value };

				// Act:
				const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

				// Assert:
				expect(format).to.deep.equal({ sub: { foo: 14, sub2: { foo: 18 } } });
			});

			it('can format nested sub object (outer object different type)', () => {
				// Arrange:
				const sample = { sub: { foo: 7, sub2: { bar: 9 } } };
				const schema = { sub: { type: SchemaType.object, schemaName: 'abc' } };
				const schemaDictionary = {
					abc: { foo: 11, sub2: { type: SchemaType.object, schemaName: 'def' } },
					def: { bar: 12 }
				};
				const formattingRules = {
					11: value => 2 * value,
					12: value => value * value
				};

				// Act:
				const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

				// Assert:
				expect(format).to.deep.equal({ sub: { foo: 14, sub2: { bar: 81 } } });
			});
		});

		describe('array', () => {
			const assertCanFormatArray = (originalArray, formattedArray, schemaName, schemaDictionary) => {
				// Arrange:
				const sample = { subarray: originalArray };
				const schema = { subarray: { type: SchemaType.array, schemaName } };
				const formattingRules = { 7: value => value[0] };

				// Act:
				const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

				// Assert:
				expect(format).to.deep.equal({ subarray: formattedArray });
			};

			it('can format empty sub array', () => {
				// Assert:
				assertCanFormatArray([], [], 'abc', { abc: { l: 7 } });
			});

			it('can format non-empty sub array of objects', () => {
				// Assert:
				assertCanFormatArray(
					[{ l: 'alpha' }, { l: 'beta' }, { l: 'gamma' }],
					[{ l: 'a' }, { l: 'b' }, { l: 'g' }],
					'abc',
					{ abc: { l: 7 } }
				);
			});

			it('can format non-empty sub array of values', () => {
				// Assert:
				assertCanFormatArray(
					['alpha', 'beta', 'gamma'],
					['a', 'b', 'g'],
					7,
					7
				);
			});
		});

		describe('dictionary', () => {
			const assertCanFormatDictionary = (originalDictionary, formattedDictionary, schemaName, schemaDictionary) => {
				// Arrange:
				const sample = { subdictionary: originalDictionary };
				const schema = { subdictionary: { type: SchemaType.dictionary, schemaName } };
				const formattingRules = { 7: value => value[0] };

				// Act:
				const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

				// Assert:
				expect(format).to.deep.equal({ subdictionary: formattedDictionary });
			};

			it('can format empty dictionary', () => {
				// Assert:
				assertCanFormatDictionary({}, {}, 'abc', { abc: { l: 7 } });
			});

			it('can format non-empty dictionary of objects', () => {
				// Assert:
				assertCanFormatDictionary(
					{ x: { l: 'alpha' }, y: { l: 'beta' }, z: { l: 'gamma' } },
					{ x: { l: 'a' }, y: { l: 'b' }, z: { l: 'g' } },
					'abc',
					{ abc: { l: 7 } }
				);
			});

			it('can format non-empty dictionary of values', () => {
				// Assert:
				assertCanFormatDictionary(
					{ x: 'alpha', y: 'beta', z: 'gamma' },
					{ x: 'a', y: 'b', z: 'g' },
					7,
					7
				);
			});
		});

		it('can use conditional schema', () => {
			// Arrange:
			const sample1 = { transaction: { type: 1, value: 5 } };
			const sample2 = { transaction: { type: 2, value: 7 } };
			const schemaDictionary = {
				type1: { value: 12345 },
				type2: { value: 54321 }
			};
			const schema = {
				transaction: {
					type: SchemaType.object,
					schemaName: entity => (1 === entity.type ? 'type1' : 'type2')
				}
			};
			const formattingRules = {
				12345: value => value + 10,
				54321: value => value + 100
			};

			// Act:
			const format1 = schemaFormatter.format(sample1, schema, schemaDictionary, formattingRules);
			const format2 = schemaFormatter.format(sample2, schema, schemaDictionary, formattingRules);

			// Assert:
			expect(format1).to.deep.equal({ transaction: { value: 15 } });
			expect(format2).to.deep.equal({ transaction: { value: 107 } });
		});

		it('can use conditional schema within arrays', () => {
			// Arrange:
			const sample = { subarray: [{ type: 1, value: 5 }, { type: 2, value: 7 }] };
			const schemaDictionary = { type1: { value: 12345 }, type2: { value: 54321 } };
			const schema = {
				subarray: {
					type: SchemaType.array,
					schemaName: entity => (1 === entity.type ? 'type1' : 'type2')
				}
			};
			const formattingRules = {
				12345: value => value + 10,
				54321: value => value + 100
			};

			// Act:
			const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

			// Assert:
			expect(format).to.deep.equal({ subarray: [{ value: 15 }, { value: 107 }] });
		});

		it('can mix multiple rules', () => {
			// Arrange:
			const sample = {
				bytes: Uint8Array.of(0xA5, 0xB2, 0x18),
				u64: [1, 2],
				other: 'abc',
				sub: { foo: 7, sub2: { foo: 9 } },
				subarray: [{ l: 'alpha' }, { l: 'beta' }, { l: 'gamma' }]
			};
			const schema = {
				bytes: 7,
				u64: 8,
				sub: { type: SchemaType.object, resultKey: 'f', schemaName: 'def' },
				subarray: { type: SchemaType.array, resultKey: 'chs', schemaName: 'abc' }
			};
			const schemaDictionary = {
				abc: { l: { type: 11, resultKey: 'ch' } },
				def: { foo: { type: 12, resultKey: 'sq' } }
			};
			const formattingRules = {
				7: value => value[0].toString(),
				8: value => value[0] + (value[1] * 10),
				11: value => value[1],
				12: value => value * value
			};

			// Act:
			const format = schemaFormatter.format(sample, schema, schemaDictionary, formattingRules);

			// Assert:
			expect(format).to.deep.equal({
				bytes: '165',
				u64: 21,
				f: { sq: 49 },
				chs: [{ ch: 'l' }, { ch: 'e' }, { ch: 'a' }]
			});
		});
	});
});
