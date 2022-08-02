const { PrivateKey } = require('../../src/CryptoTypes');
const { expect } = require('chai');

const runMessageEncoderDecodeSuccessTests = testDescriptor => {
	const testSuffix = testDescriptor.name ? ` (${testDescriptor.name})` : '';

	it(`sender can decode encoded message${testSuffix}`, () => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const recipientPublicKey = new testDescriptor.KeyPair(PrivateKey.random()).publicKey;
		const encoder = new testDescriptor.MessageEncoder(keyPair);
		const encoded = testDescriptor.encodeAccessor(encoder)(recipientPublicKey, (new TextEncoder()).encode('hello world'));

		// Act:
		const [result, decoded] = encoder.tryDecode(recipientPublicKey, encoded);

		// Assert:
		expect(result).to.equal(true);
		expect(decoded).to.deep.equal((new TextEncoder()).encode('hello world'));
	});

	it(`recipient can decode encoded message${testSuffix}`, () => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const recipientKeyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const encoder = new testDescriptor.MessageEncoder(keyPair);
		const encoded = testDescriptor.encodeAccessor(encoder)(recipientKeyPair.publicKey, (new TextEncoder()).encode('hello world'));

		// Act:
		const decoder = new testDescriptor.MessageEncoder(recipientKeyPair);
		const [result, decoded] = decoder.tryDecode(keyPair.publicKey, encoded);

		// Assert:
		expect(result).to.equal(true);
		expect(decoded).to.deep.equal((new TextEncoder()).encode('hello world'));
	});
};

const runMessageEncoderDecodeFailureTests = testDescriptor => {
	const testSuffix = testDescriptor.name ? ` (${testDescriptor.name})` : '';

	const runDecodeFailureTest = message => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const recipientPublicKey = new testDescriptor.KeyPair(PrivateKey.random()).publicKey;
		const encoder = new testDescriptor.MessageEncoder(keyPair);
		const encoded = testDescriptor.encodeAccessor(encoder)(recipientPublicKey, (new TextEncoder()).encode(message));

		testDescriptor.malformEncoded(encoded);

		// Act:
		const [result, decoded] = encoder.tryDecode(recipientPublicKey, encoded);

		// Assert:
		expect(result).to.equal(false);
		expect(decoded).to.deep.equal(encoded);
	};

	it(`decode falls back to input when decoding failed - short${testSuffix}`, () => {
		runDecodeFailureTest('hello world');
	});
	it(`decode falls back to input when decoding failed - long${testSuffix}`, () => {
		runDecodeFailureTest('bit longer message that should span upon multiple encryption blocks');
	});
};

const runBasicMessageEncoderTests = testDescriptor => {
	runMessageEncoderDecodeSuccessTests(testDescriptor);
	runMessageEncoderDecodeFailureTests(testDescriptor);
};

module.exports = { runBasicMessageEncoderTests, runMessageEncoderDecodeFailureTests };
