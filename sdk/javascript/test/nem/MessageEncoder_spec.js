import { PrivateKey, PublicKey } from '../../src/CryptoTypes.js';
import { KeyPair } from '../../src/nem/KeyPair.js';
import MessageEncoder from '../../src/nem/MessageEncoder.js';
import { Message, MessageType } from '../../src/nem/models.js';
import { runBasicMessageEncoderTests } from '../test/messageEncoderTests.js';
import { expect } from 'chai';

describe('MessageEncoder (NEM)', () => {
	runBasicMessageEncoderTests({
		KeyPair,
		MessageEncoder,
		malformEncoded: encoded => {
			encoded.message[encoded.message.length - 20] ^= 0xFF;
		}
	});

	runBasicMessageEncoderTests({
		name: 'deprecated',
		KeyPair,
		MessageEncoder,
		encodeAccessor: encoder => encoder.encodeDeprecated.bind(encoder),
		malformEncoded: encoded => {
			encoded.message[encoded.message.length - 1] ^= 0xFF;
		}
	});

	it('decode falls back to input when cbc block size is invalid', () => {
		// Arrange:
		const encoder = new MessageEncoder(new KeyPair(PrivateKey.random()));
		const recipientPublicKey = new KeyPair(PrivateKey.random()).publicKey;

		const encodedMessage = new Message();
		encodedMessage.messageType = MessageType.ENCRYPTED;
		encodedMessage.message = new Uint8Array(16 + 32 + 1);

		// Act:
		const [result, decoded] = encoder.tryDecode(recipientPublicKey, encodedMessage);

		// Assert:
		expect(result).to.equal(false);
		expect(decoded).to.deep.equal(encodedMessage);
	});

	it('decode throws when message type is invalid', () => {
		// Arrange:
		const encoder = new MessageEncoder(new KeyPair(PrivateKey.random()));

		const encodedMessage = new Message();
		encodedMessage.messageType = MessageType.PLAIN;

		// Act + Assert:
		expect(() => { encoder.tryDecode(new PublicKey(new Uint8Array(PublicKey.SIZE)), encodedMessage); })
			.to.throw('invalid message format');
	});
});
