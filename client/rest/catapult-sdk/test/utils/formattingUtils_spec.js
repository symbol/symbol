const { expect } = require('chai');
const formattingUtils = require('../../src/utils/formattingUtils');

const createEntity = seed => ({ foo: seed, bar: seed + 1, bazz: seed + 2 });

const formatter = {
	format: entity => ({
		foo: entity.foo * 2,
		bar: entity.bar * 3,
		bazz: entity.bazz * 5
	})
};

describe('formatting utils', () => {
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
