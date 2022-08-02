const { PrivateKey, PublicKey } = require('../../src/CryptoTypes');
const { concatArrays } = require('../../src/impl/CipherHelpers');
const { KeyPair } = require('../../src/symbol/KeyPair');
const { MessageEncoder } = require('../../src/symbol/MessageEncoder');
const { runBasicMessageEncoderTests, runMessageEncoderDecodeFailureTests } = require('../test/messageEncoderTests');
const { expect } = require('chai');

describe('MessageEncoder (Symbol)', () => {
	runBasicMessageEncoderTests({
		KeyPair,
		MessageEncoder,
		encodeAccessor: encoder => encoder.encode.bind(encoder),
		malformEncoded: encoded => {
			encoded[encoded.length - 1] ^= 0xFF;
		}
	});

	runMessageEncoderDecodeFailureTests({
		name: 'fake delegation',
		KeyPair,
		MessageEncoder,
		encodeAccessor: encoder => (publicKey, message) => {
			const result = encoder.encode(publicKey, message);
			const fakeMarker = Uint8Array.from(Buffer.from('FE2A8061577301E2', 'hex'));
			return concatArrays(fakeMarker, publicKey.bytes, result.subarray(1));
		},
		malformEncoded: encoded => {
			encoded[encoded.length - 1] ^= 0xFF;
		}
	});

	// note: there's no sender decode test for persistent harvesting delegation, cause sender does not have ephemeral key pair
	it('recipient can decode encoded persistent harvesting delegation', () => {
		// Arrange:
		const keyPair = new KeyPair(PrivateKey.random());
		const nodeKeyPair = new KeyPair(PrivateKey.random());
		const remoteKeyPair = new KeyPair(new PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'));
		const vrfKeyPair = new KeyPair(new PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'));
		const encoder = new MessageEncoder(keyPair);
		const encoded = encoder.encodePersistentHarvestingDelegation(nodeKeyPair.publicKey, remoteKeyPair, vrfKeyPair);

		// Act:
		const decoder = new MessageEncoder(nodeKeyPair);
		const [result, decoded] = decoder.tryDecode(keyPair.publicKey, encoded);

		// Assert:
		expect(result).to.equal(true);
		expect(decoded).to.deep.equal(concatArrays(remoteKeyPair.privateKey.bytes, vrfKeyPair.privateKey.bytes));
	});

	it('decode falls back to input when ephemeral public key is not valid', () => {
		// Arrange: create valid persistent harvesting delegation request
		const keyPair = new KeyPair(PrivateKey.random());
		const nodeKeyPair = new KeyPair(PrivateKey.random());
		const remoteKeyPair = new KeyPair(PrivateKey.random());
		const vrfKeyPair = new KeyPair(PrivateKey.random());
		const encoder = new MessageEncoder(keyPair);
		const encoded = encoder.encodePersistentHarvestingDelegation(nodeKeyPair.publicKey, remoteKeyPair, vrfKeyPair);

		// - zero public key
		for (let i = 8; i < 8 + 32; ++i)
			encoded[i] = 0;

		// Act:
		const decoder = new MessageEncoder(nodeKeyPair);
		const [result, decoded] = decoder.tryDecode(keyPair.publicKey, encoded);

		// Assert:
		expect(result).to.equal(false);
		expect(decoded).to.deep.equal(encoded);
	});

	it('decode falls back to input when message has unknown type', () => {
		// Arrange:
		const encoder = new MessageEncoder(new KeyPair(PrivateKey.random()));
		const invalidEncoded = Uint8Array.from(Buffer.from('024A4A4A', 'hex'));

		// Act:
		const [result, decoded] = encoder.tryDecode(new PublicKey(new Uint8Array(32)), invalidEncoded);

		// Assert:
		expect(result).to.equal(false);
		expect(decoded).to.deep.equal(invalidEncoded);
	});
});
