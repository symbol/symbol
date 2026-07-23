package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.charset.StandardCharsets;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;
import org.symbol.sdk.nem.models.*;

/**
 * Tests {@link MessageEncoder} encode/decode round-trips for AES-GCM and the deprecated AES-CBC formats. Mirrors
 * {@code test/nem/MessageEncoder_spec.js}, which runs the shared {@code messageEncoderTests.js} suite twice (recommended + deprecated); the
 * "(deprecated)" suffixed tests below correspond to the second run.
 */
final class MessageEncoderTest {

	private static final byte[] HELLO_WORLD = "hello world".getBytes(StandardCharsets.UTF_8);

	private static final byte[] LONG_MESSAGE = "bit longer message that should span upon multiple encryption blocks"
			.getBytes(StandardCharsets.UTF_8);

	@FunctionalInterface
	private interface EncodeAccessor {
		Message encode(MessageEncoder encoder, CryptoTypes.PublicKey recipientPublicKey, byte[] message);
	}

	// region shared suite - recommended (AES-GCM)

	@Test
	void canCreateEncoder() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());

		// Act:
		final MessageEncoder encoder = new MessageEncoder(keyPair);

		// Assert:
		assertThat(encoder.getPublicKey(), is(equalTo(keyPair.getPublicKey())));
	}

	@Test
	void senderCanDecodeEncodedMessage() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final Message encoded = encoder.encode(recipientPublicKey, HELLO_WORLD);

		// Act:
		final MessageEncoderResult result = encoder.tryDecode(recipientPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	@Test
	void recipientCanDecodeEncodedMessage() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final KeyPair recipientKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final Message encoded = encoder.encode(recipientKeyPair.getPublicKey(), HELLO_WORLD);

		// Act:
		final MessageEncoder decoder = new MessageEncoder(recipientKeyPair);
		final MessageEncoderResult result = decoder.tryDecode(keyPair.getPublicKey(), encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	@Test
	void decodeFallsBackToInputWhenDecodingFailedShort() {
		assertDecodeFallsBackToInput(HELLO_WORLD, MessageEncoder::encode, 20);
	}

	@Test
	void decodeFallsBackToInputWhenDecodingFailedLong() {
		assertDecodeFallsBackToInput(LONG_MESSAGE, MessageEncoder::encode, 20);
	}

	// endregion

	// region shared suite - deprecated (AES-CBC)

	@Test
	void senderCanDecodeEncodedMessageDeprecated() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final Message encoded = encoder.encodeDeprecated(recipientPublicKey, HELLO_WORLD);

		// Act:
		final MessageEncoderResult result = encoder.tryDecode(recipientPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	@Test
	void recipientCanDecodeEncodedMessageDeprecated() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final KeyPair recipientKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final Message encoded = encoder.encodeDeprecated(recipientKeyPair.getPublicKey(), HELLO_WORLD);

		// Act:
		final MessageEncoder decoder = new MessageEncoder(recipientKeyPair);
		final MessageEncoderResult result = decoder.tryDecode(keyPair.getPublicKey(), encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	@Test
	void decodeFallsBackToInputWhenDecodingFailedShortDeprecated() {
		assertDecodeFallsBackToInput(HELLO_WORLD, MessageEncoder::encodeDeprecated, 1);
	}

	@Test
	void decodeFallsBackToInputWhenDecodingFailedLongDeprecated() {
		// Arrange + Act + Assert:
		assertDecodeFallsBackToInput(LONG_MESSAGE, MessageEncoder::encodeDeprecated, 1);
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
		assertThat(result.message() == encoded, is(true));
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

	// region helpers

	private static void assertDecodeFallsBackToInput(final byte[] message, final EncodeAccessor encodeAccessor,
			final int malformOffsetFromEnd) {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final Message encoded = encodeAccessor.encode(encoder, recipientPublicKey, message);

		final byte[] payload = encoded.getMessage();
		payload[payload.length - malformOffsetFromEnd] ^= (byte) 0xFF;
		encoded.setMessage(payload);

		// Act:
		final MessageEncoderResult result = encoder.tryDecode(recipientPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(false));
		assertThat(result.message() == encoded, is(true));
	}

	// endregion
}
