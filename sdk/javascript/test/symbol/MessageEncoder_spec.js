import { PrivateKey, PublicKey } from '../../src/CryptoTypes.js';
import { concatArrays } from '../../src/impl/CipherHelpers.js';
import { KeyPair } from '../../src/symbol/KeyPair.js';
import MessageEncoder from '../../src/symbol/MessageEncoder.js';
import { runBasicMessageEncoderTests, runMessageEncoderDecodeFailureTests } from '../test/messageEncoderTests.js';
import { expect } from 'chai';

describe('MessageEncoder (Symbol)', () => {
	it('can create encoder', () => {
		// Arrange:
		const keyPair = new KeyPair(PrivateKey.random());

		// Act:
		const encoder = new MessageEncoder(keyPair);

		// Assert:
		expect(encoder.publicKey).to.deep.equal(keyPair.publicKey);
	});

	const malformEncoded = encoded => {
		encoded[encoded.length - 1] ^= 0xFF;
	};

	describe('recommended', () => {
		runBasicMessageEncoderTests({
			KeyPair,
			MessageEncoder,
			malformEncoded
		});

		it('decode falls back to input when message has unknown type', () => {
			// Arrange:
			const encoder = new MessageEncoder(new KeyPair(PrivateKey.random()));
			const invalidEncoded = Uint8Array.from(Buffer.from('024A4A4A', 'hex'));

			// Act:
			const result = encoder.tryDecode(new PublicKey(new Uint8Array(32)), invalidEncoded);

			// Assert:
			expect(result.isDecoded).to.equal(false);
			expect(result.message).to.deep.equal(invalidEncoded);
		});
	});

	describe('delegation', () => {
		runMessageEncoderDecodeFailureTests({
			KeyPair,
			MessageEncoder,
			encodeAccessor: encoder => () => {
				// simulate a delegation message where node and ephemeral key pairs are used
				// for these tests to work properly, the encoder key pair is used as the node key pair
				const remoteKeyPair = new KeyPair(new PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'));
				const vrfKeyPair = new KeyPair(new PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'));
				return encoder.encodePersistentHarvestingDelegation(encoder.publicKey, remoteKeyPair, vrfKeyPair);
			},
			malformEncoded
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
			const result = decoder.tryDecode(keyPair.publicKey, encoded);

			// Assert:
			expect(result.isDecoded).to.equal(true);
			expect(result.message).to.deep.equal(concatArrays(remoteKeyPair.privateKey.bytes, vrfKeyPair.privateKey.bytes));
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
			const result = decoder.tryDecode(keyPair.publicKey, encoded);

			// Assert:
			expect(result.isDecoded).to.equal(false);
			expect(result.message).to.deep.equal(encoded);
		});
	});

	describe('deprecated', () => {
		runBasicMessageEncoderTests({
			KeyPair,
			MessageEncoder,
			encodeAccessor: encoder => encoder.encodeDeprecated.bind(encoder),
			tryDecodeAccessor: decoder => decoder.tryDecodeDeprecated.bind(decoder),
			malformEncoded
		});

		it('falls back to decode on failure', () => {
			// Arrange: encode using non-deprecated function
			const keyPair = new KeyPair(PrivateKey.random());
			const recipientPublicKey = new KeyPair(PrivateKey.random()).publicKey;
			const encoder = new MessageEncoder(keyPair);
			const encoded = encoder.encode(recipientPublicKey, new TextEncoder().encode('hello world'));

			// Act: decode using deprecated function
			const result = encoder.tryDecodeDeprecated(recipientPublicKey, encoded);

			// Assert: decode was successful
			expect(result.isDecoded).to.equal(true);
			expect(result.message).to.deep.equal(new TextEncoder().encode('hello world'));
		});
	});
});
