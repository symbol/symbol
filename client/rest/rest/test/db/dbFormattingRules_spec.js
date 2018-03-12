const { expect } = require('chai');
const catapult = require('catapult-sdk');
const formattingRules = require('../../src/db/dbFormattingRules');
const test = require('../testUtils');

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

	it('can format uint64 type', () => {
		// Arrange:
		const object = test.factory.createLong(1, 2);

		// Act:
		const result = formattingRules[ModelType.uint64](object);

		// Assert:
		expect(result).to.deep.equal([1, 2]);
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
		[0x80530008, -2142044152].forEach(code => {
			// Act:
			const result = formattingRules[ModelType.statusCode](code);

			// Assert:
			expect(result, `${code} code`).to.equal('Failure_Signature_Not_Verifiable');
		});
	});
});
