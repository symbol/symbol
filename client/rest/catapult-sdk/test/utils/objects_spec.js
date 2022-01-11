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

const objects = require('../../src/utils/objects');
const { expect } = require('chai');

describe('objects', () => {
	const propertyTypeDescriptors = [
		{ name: 'undefined', value: undefined },
		{ name: 'null', value: null },
		{ name: 'numeric', value: 7 }, // sample of a pod (something that does not have type 'object')
		{ name: 'object', value: { color: 'crimson' } },
		{ name: 'array', value: [4, 9, 16] }
	];

	describe('deepAssign', () => {
		const addSimpleTest = (name, target, source) => {
			it(name, () => {
				// Act:
				const result = objects.deepAssign({ foo: target }, { foo: source });

				// Assert:
				expect(result).to.deep.equal({ foo: source });
			});
		};

		addSimpleTest('pod can replace pod of same type', 9, 4);

		const addLastPropertyTest = (description, value) => {
			it(`last object property takes precedence (${description})`, () => {
				// Act:
				const result = objects.deepAssign({ name: 'alpha' }, { name: 'beta' }, { name: value });

				// Assert: the value from the last object passed to deepAssign is used
				expect(result).to.deep.equal({ name: value });
			});
		};

		addLastPropertyTest('non-null', 'gamma');
		addLastPropertyTest('null', null);
		addLastPropertyTest('undefined', undefined);

		// add tests for all descriptor combinations ensuring that any property type can replace any other property type
		propertyTypeDescriptors.forEach(descriptor1 => {
			it(`${descriptor1.name} can replace other types`, () => {
				// Arrange:
				let numTests = 0;
				propertyTypeDescriptors.forEach(descriptor2 => {
					if (descriptor1 !== descriptor2) {
						// Act:
						const result = objects.deepAssign({ foo: descriptor2.value }, { foo: descriptor1.value });

						// Assert:
						expect(result).to.deep.equal({ foo: descriptor1.value });
						++numTests;
					}
				});

				// Sanity: one iteration for each different type descriptor
				expect(numTests).to.equal(4);
			});
		});

		it('unique properties are merged', () => {
			// Act:
			const result = objects.deepAssign({ a: 1, b: 2, c: 3 }, { d: 4, e: 5 }, { f: 6 });

			// Assert:
			expect(result).to.deep.equal({
				a: 1,
				b: 2,
				c: 3,
				d: 4,
				e: 5,
				f: 6
			});
		});

		it('unique subobject properties are merged', () => {
			// Act:
			const result = objects.deepAssign({ foo: { a: 1, b: 2, c: 3 } }, { foo: { d: 4, e: 5 } }, { foo: { f: 6 } });

			// Assert:
			expect(result).to.deep.equal({
				foo: {
					a: 1,
					b: 2,
					c: 3,
					d: 4,
					e: 5,
					f: 6
				}
			});
		});

		it('rightmost property values are used', () => {
			// Act:
			const result = objects.deepAssign({ a: 1, b: 2, c: 3 }, { b: 4, c: 5 }, { b: 6 });

			// Assert:
			expect(result).to.deep.equal({
				a: 1,
				b: 6,
				c: 5
			});
		});

		it('rightmost subobject property values are used', () => {
			// Act:
			const result = objects.deepAssign({ foo: { a: 1, b: 2, c: 3 } }, { foo: { b: 4, c: 5 } }, { foo: { b: 6 } });

			// Assert:
			expect(result).to.deep.equal({
				foo: {
					a: 1,
					b: 6,
					c: 5
				}
			});
		});

		addSimpleTest('shorter array can replace longer array', [1, 2, 6], [4, 9]);
		addSimpleTest('longer array can replace shorter array', [4, 9], [1, 2, 6]);
	});

	describe('check schema against template', () => {
		const createUnknownPropertyMessage = propertyName => `unknown '${propertyName}' key in config`;

		const createMistypedPropertyMessage = propertyName => `override '${propertyName}' property has wrong type`;

		it('does not allow undefined property to be replaced', () => {
			// Arrange:
			let numTests = 0;
			propertyTypeDescriptors.forEach(descriptor => {
				// Act + Assert:
				const message = `${descriptor.name} cannot replace undefined`;
				expect(() => objects.checkSchemaAgainstTemplate({ foo: undefined }, { foo: descriptor.value }), message)
					.to.throw(createUnknownPropertyMessage('foo'));
				++numTests;
			});

			// Sanity:
			expect(numTests).to.equal(5);
		});

		it('does not allow property to be replaced by different type', () => {
			// Arrange:
			let numTests = 0;
			propertyTypeDescriptors.forEach(descriptor1 => propertyTypeDescriptors.forEach(descriptor2 => {
				if (undefined !== descriptor1.value && descriptor1 !== descriptor2) {
					// Act + Assert:
					const message = `${descriptor2.name} cannot replace ${descriptor1.name}`;
					expect(() => objects.checkSchemaAgainstTemplate({ foo: descriptor1.value }, { foo: descriptor2.value }), message)
						.to.throw(createMistypedPropertyMessage('foo'));
					++numTests;
				}
			}));

			// Sanity:
			expect(numTests).to.equal(16);
		});

		const addCompatibleTest = (name, target, source) => {
			it(name, () => {
				// Act + Assert:
				expect(() => objects.checkSchemaAgainstTemplate({ foo: target }, { foo: source })).to.not.throw();
			});
		};

		addCompatibleTest('allows null to be replaced by null', null, null);
		addCompatibleTest('allows numeric to be replaced by numeric', 8, 7);
		addCompatibleTest('allows object to be replaced by object', { foo: 6 }, { foo: 9 });
		addCompatibleTest('allows array to be replaced by array', [7, 5], [1, 2, 6]);

		it('does not allow subobject property to be replaced by different type', () => {
			// Act + Assert:
			expect(() => objects.checkSchemaAgainstTemplate({ foo: { bar: 6 } }, { foo: { bar: 'red' } }))
				.to.throw(createMistypedPropertyMessage('bar'));
		});

		it('allows template-only property', () => {
			// Act + Assert: bar is only in template
			expect(() => objects.checkSchemaAgainstTemplate({ foo: 6, bar: 8 }, { foo: 7 })).to.not.throw();
		});

		it('does not allow override-only property', () => {
			// Act + Assert: bar is only in override object
			expect(() => objects.checkSchemaAgainstTemplate({ foo: 6 }, { foo: 7, bar: 8 }))
				.to.throw(createUnknownPropertyMessage('bar'));
		});
	});
});
