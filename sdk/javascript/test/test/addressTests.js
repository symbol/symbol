import { expect } from 'chai';

export const runBasicAddressTests = testDescriptor => { // eslint-disable-line import/prefer-default-export
	it('has correct size', () => {
		expect(testDescriptor.Address.SIZE).to.equal(testDescriptor.decodedAddress.length);
	});

	it('can create from address', () => {
		// Arrange:
		const originalAddress = new testDescriptor.Address(testDescriptor.encodedAddress);

		// Act:
		const address = new testDescriptor.Address(originalAddress);

		// Assert:
		expect(address.bytes).to.deep.equal(testDescriptor.decodedAddress);
		expect(address.toString()).to.equal(testDescriptor.encodedAddress);
	});

	it('can create from encoded address', () => {
		// Act:
		const address = new testDescriptor.Address(testDescriptor.encodedAddress);

		// Assert:
		expect(address.bytes).to.deep.equal(testDescriptor.decodedAddress);
		expect(address.toString()).to.equal(testDescriptor.encodedAddress);
	});

	it('can create from decoded address', () => {
		// Act:
		const address = new testDescriptor.Address(testDescriptor.decodedAddress);

		// Assert:
		expect(address.bytes).to.deep.equal(testDescriptor.decodedAddress);
		expect(address.toString()).to.equal(testDescriptor.encodedAddress);
	});
};
