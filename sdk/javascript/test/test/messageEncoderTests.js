import { PrivateKey } from '../../src/CryptoTypes.js';
import { expect } from 'chai';

const encode = (testDescriptor, encoder) => (
	testDescriptor.encodeAccessor ? testDescriptor.encodeAccessor(encoder) : encoder.encode.bind(encoder)
);
const tryDecode = (testDescriptor, decoder) => (
	testDescriptor.tryDecodeAccessor ? testDescriptor.tryDecodeAccessor(decoder) : decoder.tryDecode.bind(decoder)
);

const runMessageEncoderDecodeSuccessTests = testDescriptor => {
	const testSuffix = testDescriptor.name ? ` (${testDescriptor.name})` : '';

	it(`sender can decode encoded message${testSuffix}`, () => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const recipientPublicKey = new testDescriptor.KeyPair(PrivateKey.random()).publicKey;
		const encoder = new testDescriptor.MessageEncoder(keyPair);
		const encoded = encode(testDescriptor, encoder)(recipientPublicKey, (new TextEncoder()).encode('hello world'));

		// Act:
		const result = tryDecode(testDescriptor, encoder)(recipientPublicKey, encoded);

		// Assert:
		expect(result.isDecoded).to.equal(true);
		expect(result.message).to.deep.equal((new TextEncoder()).encode('hello world'));
	});

	it(`recipient can decode encoded message${testSuffix}`, () => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const recipientKeyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const encoder = new testDescriptor.MessageEncoder(keyPair);
		const encoded = encode(testDescriptor, encoder)(recipientKeyPair.publicKey, (new TextEncoder()).encode('hello world'));

		// Act:
		const decoder = new testDescriptor.MessageEncoder(recipientKeyPair);
		const result = tryDecode(testDescriptor, decoder)(keyPair.publicKey, encoded);

		// Assert:
		expect(result.isDecoded).to.equal(true);
		expect(result.message).to.deep.equal((new TextEncoder()).encode('hello world'));
	});
};

export const runMessageEncoderDecodeFailureTests = testDescriptor => {
	const testSuffix = testDescriptor.name ? ` (${testDescriptor.name})` : '';

	const runDecodeFailureTest = message => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const recipientPublicKey = new testDescriptor.KeyPair(PrivateKey.random()).publicKey;
		const encoder = new testDescriptor.MessageEncoder(keyPair);
		const encoded = encode(testDescriptor, encoder)(recipientPublicKey, (new TextEncoder()).encode(message));

		testDescriptor.malformEncoded(encoded);

		// Act:
		const result = tryDecode(testDescriptor, encoder)(recipientPublicKey, encoded);

		// Assert:
		expect(result.isDecoded).to.equal(false);
		expect(result.message).to.deep.equal(encoded);
	};

	it(`decode falls back to input when decoding failed - short${testSuffix}`, () => {
		runDecodeFailureTest('hello world');
	});
	it(`decode falls back to input when decoding failed - long${testSuffix}`, () => {
		runDecodeFailureTest('bit longer message that should span upon multiple encryption blocks');
	});
};

export const runBasicMessageEncoderTests = testDescriptor => {
	runMessageEncoderDecodeSuccessTests(testDescriptor);
	runMessageEncoderDecodeFailureTests(testDescriptor);
};
