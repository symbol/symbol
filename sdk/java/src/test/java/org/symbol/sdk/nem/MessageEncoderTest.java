package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;
import org.symbol.sdk.nem.models.*;
import org.symbol.sdk.test.AbstractMessageEncoderTest;

/**
 * Tests {@link MessageEncoder} encode/decode round-trips for AES-GCM and the deprecated AES-CBC formats: the shared encoder contract runs
 * via {@link AbstractMessageEncoderTest} per variant, plus the NEM-specific edge cases.
 */
final class MessageEncoderTest {

	private static void malformEncodedAt(final Message encoded, final int offsetFromEnd) {
		final byte[] payload = encoded.getMessage();
		payload[payload.length - offsetFromEnd] ^= (byte) 0xFF;
		encoded.setMessage(payload);
	}

	private abstract class BasicMessageEncoderTest extends AbstractMessageEncoderTest<KeyPair, MessageEncoder, Message> {
		@Override
		protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
			return new KeyPair(privateKey);
		}

		@Override
		protected MessageEncoder createEncoder(final KeyPair keyPair) {
			return new MessageEncoder(keyPair);
		}

		@Override
		protected CryptoTypes.PublicKey publicKeyOf(final MessageEncoder encoder) {
			return encoder.getPublicKey();
		}

		@Override
		protected MessageEncoderResult tryDecode(final MessageEncoder encoder, final CryptoTypes.PublicKey publicKey,
				final Message encoded) {
			return encoder.tryDecode(publicKey, encoded);
		}

		@Override
		protected byte[] encodedToBytes(final Message encoded) {
			return encoded.serialize();
		}

		@Override
		protected Message toEncoded(final Object message) {
			return (Message) message;
		}
	}

	// region recommended (AES-GCM)

	@Nested
	final class Recommended extends BasicMessageEncoderTest {
		@Override
		protected Message encode(final MessageEncoder encoder, final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
			return encoder.encode(recipientPublicKey, message);
		}

		@Override
		protected void malformEncoded(final Message encoded) {
			malformEncodedAt(encoded, 20);
		}
	}

	// endregion

	// region deprecated (AES-CBC)

	@Nested
	final class Deprecated extends BasicMessageEncoderTest {
		@Override
		protected Message encode(final MessageEncoder encoder, final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
			return encoder.encodeDeprecated(recipientPublicKey, message);
		}

		@Override
		protected void malformEncoded(final Message encoded) {
			malformEncodedAt(encoded, 1);
		}
	}

	// endregion

	// region edge cases

	@Test
	void decodeFallsBackToInputWhenCbcBlockSizeIsInvalid() {
		// Arrange:
		final MessageEncoder encoder = new MessageEncoder(new KeyPair(CryptoTypes.PrivateKey.random()));
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();

		final Message encoded = new Message();
		encoded.setMessageType(MessageType.ENCRYPTED);
		encoded.setMessage(new byte[16 + 32 + 1]);

		// Act:
		final MessageEncoderResult result = encoder.tryDecode(recipientPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(false));
		assertThat(((Message) result.message()).serialize(), is(equalTo(encoded.serialize())));
	}

	@Test
	void decodeThrowsWhenCbcPayloadIsShorterThanSalt() {
		// Arrange: too short for the 32-byte salt (and already rejected by the GCM path)
		final MessageEncoder encoder = new MessageEncoder(new KeyPair(CryptoTypes.PrivateKey.random()));
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();

		final Message encoded = new Message();
		encoded.setMessageType(MessageType.ENCRYPTED);
		encoded.setMessage(new byte[31]);

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
				() -> encoder.tryDecode(recipientPublicKey, encoded));

		// Assert:
		assertThat(ex.getMessage(), is(equalTo("invalid salt")));
	}

	@Test
	void decodeThrowsWhenMessageTypeIsInvalid() {
		// Arrange:
		final MessageEncoder encoder = new MessageEncoder(new KeyPair(CryptoTypes.PrivateKey.random()));
		final Message encoded = new Message();
		encoded.setMessageType(MessageType.PLAIN);

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
				() -> encoder.tryDecode(new CryptoTypes.PublicKey(new byte[CryptoTypes.PublicKey.SIZE]), encoded));

		// Assert:
		assertThat(ex.getMessage(), is(equalTo("invalid message format")));
	}

	// endregion
}
