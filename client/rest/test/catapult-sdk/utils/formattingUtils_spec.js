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

const formattingUtils = require('../../../src/catapult-sdk/utils/formattingUtils');
const { expect } = require('chai');

const createEntity = seed => ({ foo: seed, bar: seed + 1, bazz: seed + 2 });

const formatter = {
	format: entity => ({
		foo: entity.foo * 2,
		bar: entity.bar * 3,
		bazz: entity.bazz * 5
	})
};

describe('formatting utils', () => {
	describe('format array', () => {
		it('can format empty array', () => {
			// Act:
			const formattedResult = formattingUtils.formatArray(formatter, []);

			// Assert:
			expect(formattedResult).to.deep.equal([]);
		});

		it('can format array with single element', () => {
			// Arrange:
			const entity = createEntity(5);

			// Act:
			const formattedResult = formattingUtils.formatArray(formatter, [entity]);

			// Assert:
			expect(formattedResult).to.deep.equal([{ foo: 10, bar: 18, bazz: 35 }]);
		});

		it('can format array with multiple elements', () => {
			// Arrange:
			const entity1 = createEntity(2);
			const entity2 = createEntity(5);
			const entity3 = createEntity(11);

			// Act:
			const formattedResult = formattingUtils.formatArray(formatter, [entity1, entity2, entity3]);

			// Assert:
			expect(formattedResult).to.deep.equal([
				{ foo: 4, bar: 9, bazz: 20 },
				{ foo: 10, bar: 18, bazz: 35 },
				{ foo: 22, bar: 36, bazz: 65 }]);
		});
	});

	describe('format page', () => {
		it('can format empty page', () => {
			// Act:
			const formattedResult = formattingUtils.formatPage(formatter, { data: [], pagination: {} });

			// Assert:
			expect(formattedResult).to.deep.equal({ data: [], pagination: {} });
		});

		it('can format page with single element', () => {
			// Arrange:
			const entity = createEntity(5);
			const paginatedEntity = {
				data: [entity],
				pagination: { numberOfEntities: 1 }
			};

			// Act:
			const formattedResult = formattingUtils.formatPage(formatter, paginatedEntity);

			// Assert:
			expect(formattedResult).to.deep.equal({
				data: [{ foo: 10, bar: 18, bazz: 35 }],
				pagination: { numberOfEntities: 1 }
			});
		});

		it('can format page with multiple elements', () => {
			// Arrange:
			const paginatedEntity = {
				data: [createEntity(2), createEntity(5), createEntity(11)],
				pagination: { numberOfEntities: 3 }
			};

			// Act:
			const formattedResult = formattingUtils.formatPage(formatter, paginatedEntity);

			// Assert:
			expect(formattedResult).to.deep.equal({
				data: [
					{ foo: 4, bar: 9, bazz: 20 },
					{ foo: 10, bar: 18, bazz: 35 },
					{ foo: 22, bar: 36, bazz: 65 }
				],
				pagination: { numberOfEntities: 3 }
			});
		});
	});
});
