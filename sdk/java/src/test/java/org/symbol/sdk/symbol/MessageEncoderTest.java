package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;

/**
 * Tests {@link MessageEncoder}: recommended encode/decode round-trips plus the delegation and deprecated wallet flows.
 */
final class MessageEncoderTest {

	private static final byte[] HELLO_WORLD = "hello world".getBytes(StandardCharsets.UTF_8);

	private static final byte[] LONG_MESSAGE = "bit longer message that should span upon multiple encryption blocks"
			.getBytes(StandardCharsets.UTF_8);

	// region shared test bodies (mirror of JS test/test/messageEncoderTests.js)

	@FunctionalInterface
	private interface EncodeFunction {
		byte[] encode(MessageEncoder encoder, CryptoTypes.PublicKey recipientPublicKey, byte[] message);
	}

	@FunctionalInterface
	private interface TryDecodeFunction {
		MessageEncoderResult tryDecode(MessageEncoder decoder, CryptoTypes.PublicKey publicKey, byte[] encoded);
	}

	private static void malformEncoded(final byte[] encoded) {
		encoded[encoded.length - 1] ^= (byte) 0xFF;
	}

	// simulates a delegation message where node and ephemeral key pairs are used; for the delegation failure tests to work properly,
	// the encoder key pair is used as the node key pair (the recipient public key and message arguments are ignored, like in JS)
	private static byte[] encodeDelegation(final MessageEncoder encoder, final CryptoTypes.PublicKey recipientPublicKey,
			final byte[] message) {
		final KeyPair remoteKeyPair = new KeyPair(
				new CryptoTypes.PrivateKey("11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF"));
		final KeyPair vrfKeyPair = new KeyPair(
				new CryptoTypes.PrivateKey("11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF"));
		return encoder.encodePersistentHarvestingDelegation(encoder.getPublicKey(), remoteKeyPair, vrfKeyPair);
	}

	private static void assertCanCreateEncoder() {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());

		// Act:
		final MessageEncoder encoder = new MessageEncoder(keyPair);

		// Assert:
		assertThat(encoder.getPublicKey(), is(equalTo(keyPair.getPublicKey())));
	}

	private static void assertSenderCanDecodeEncodedMessage(final EncodeFunction encode, final TryDecodeFunction tryDecode) {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final byte[] encoded = encode.encode(encoder, recipientPublicKey, HELLO_WORLD);

		// Act:
		final MessageEncoderResult result = tryDecode.tryDecode(encoder, recipientPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	private static void assertRecipientCanDecodeEncodedMessage(final EncodeFunction encode, final TryDecodeFunction tryDecode) {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final KeyPair recipientKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final byte[] encoded = encode.encode(encoder, recipientKeyPair.getPublicKey(), HELLO_WORLD);

		// Act:
		final MessageEncoder decoder = new MessageEncoder(recipientKeyPair);
		final MessageEncoderResult result = tryDecode.tryDecode(decoder, keyPair.getPublicKey(), encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	private static void assertDecodeFallsBackToInputWhenDecodingFailed(final byte[] message, final EncodeFunction encode,
			final TryDecodeFunction tryDecode) {
		// Arrange:
		final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
		final MessageEncoder encoder = new MessageEncoder(keyPair);
		final byte[] encoded = encode.encode(encoder, recipientPublicKey, message);

		malformEncoded(encoded);

		// Act:
		final MessageEncoderResult result = tryDecode.tryDecode(encoder, recipientPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(false));
		assertThat((byte[]) result.message(), is(equalTo(encoded)));
	}

	// endregion

	// region recommended

	@Nested
	final class Recommended {
		@Test
		void canCreateEncoder() {
			// Arrange + Act + Assert:
			assertCanCreateEncoder();
		}

		@Test
		void senderCanDecodeEncodedMessage() {
			// Arrange + Act + Assert:
			assertSenderCanDecodeEncodedMessage(MessageEncoder::encode, MessageEncoder::tryDecode);
		}

		@Test
		void recipientCanDecodeEncodedMessage() {
			// Arrange + Act + Assert:
			assertRecipientCanDecodeEncodedMessage(MessageEncoder::encode, MessageEncoder::tryDecode);
		}

		@Test
		void decodeFallsBackToInputWhenDecodingFailedShort() {
			// Arrange + Act + Assert:
			assertDecodeFallsBackToInputWhenDecodingFailed(HELLO_WORLD, MessageEncoder::encode, MessageEncoder::tryDecode);
		}

		@Test
		void decodeFallsBackToInputWhenDecodingFailedLong() {
			// Arrange + Act + Assert:
			assertDecodeFallsBackToInputWhenDecodingFailed(LONG_MESSAGE, MessageEncoder::encode, MessageEncoder::tryDecode);
		}

		@Test
		void decodeFallsBackToInputWhenMessageHasUnknownType() {
			// Arrange:
			final MessageEncoder encoder = new MessageEncoder(new KeyPair(CryptoTypes.PrivateKey.random()));
			final byte[] invalidEncoded = new byte[]{
					0x02, 0x4A, 0x4A, 0x4A
			};

			// Act:
			final MessageEncoderResult result = encoder.tryDecode(new CryptoTypes.PublicKey(new byte[32]), invalidEncoded);

			// Assert:
			assertThat(result.isDecoded(), is(false));
			assertThat((byte[]) result.message(), is(equalTo(invalidEncoded)));
		}

		@Test
		void tryDecodeRejectsOffCurveKeyEvenWhenPayloadIsShort() {
			// Arrange: an off-curve recipient key must be rejected during key derivation ("invalid point"), not silently
			// reported as not-decoded, even when the payload is too short to be a GCM message
			final MessageEncoder encoder = new MessageEncoder(new KeyPair(CryptoTypes.PrivateKey.random()));
			final byte[] offCurveKey = new byte[32];
			Arrays.fill(offCurveKey, (byte) 0xFF);
			final byte[] shortGcmMessage = {
					1, 2, 3 // 0x01 prefix, payload shorter than TAG_SIZE + GCM_IV_SIZE
			};

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> encoder.tryDecode(new CryptoTypes.PublicKey(offCurveKey), shortGcmMessage));
		}

		@Test
		void emptyEncodedRejectedGracefully() {
			// Arrange:
			final MessageEncoder encoder = new MessageEncoder(new KeyPair(CryptoTypes.PrivateKey.random()));

			// Act:
			final MessageEncoderResult result = encoder.tryDecode(new CryptoTypes.PublicKey(new byte[32]), new byte[0]);

			// Assert:
			assertThat(result.isDecoded(), is(false));
		}
	}

	// endregion

	// region delegation

	@Nested
	final class Delegation {
		@Test
		void decodeFallsBackToInputWhenDecodingFailedShort() {
			// Arrange + Act + Assert:
			assertDecodeFallsBackToInputWhenDecodingFailed(HELLO_WORLD, MessageEncoderTest::encodeDelegation, MessageEncoder::tryDecode);
		}

		@Test
		void decodeFallsBackToInputWhenDecodingFailedLong() {
			// Arrange + Act + Assert:
			assertDecodeFallsBackToInputWhenDecodingFailed(LONG_MESSAGE, MessageEncoderTest::encodeDelegation, MessageEncoder::tryDecode);
		}

		// note: there's no sender decode test for persistent harvesting delegation, cause sender does not have ephemeral key pair

		@Test
		void recipientCanDecodeEncodedPersistentHarvestingDelegation() {
			// Arrange:
			final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final KeyPair nodeKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final KeyPair remoteKeyPair = new KeyPair(
					new CryptoTypes.PrivateKey("11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF"));
			final KeyPair vrfKeyPair = new KeyPair(
					new CryptoTypes.PrivateKey("11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF"));
			final MessageEncoder encoder = new MessageEncoder(keyPair);
			final byte[] encoded = encoder.encodePersistentHarvestingDelegation(nodeKeyPair.getPublicKey(), remoteKeyPair, vrfKeyPair);

			// Act:
			final MessageEncoder decoder = new MessageEncoder(nodeKeyPair);
			final MessageEncoderResult result = decoder.tryDecode(keyPair.getPublicKey(), encoded);

			// Assert:
			final byte[] expected = new byte[64];
			System.arraycopy(remoteKeyPair.getPrivateKey().bytes(), 0, expected, 0, 32);
			System.arraycopy(vrfKeyPair.getPrivateKey().bytes(), 0, expected, 32, 32);
			assertThat(result.isDecoded(), is(true));
			assertThat((byte[]) result.message(), is(equalTo(expected)));
		}

		@Test
		void decodeFallsBackToInputWhenEphemeralPublicKeyIsNotValid() {
			// Arrange: create valid persistent harvesting delegation request
			final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final KeyPair nodeKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final KeyPair remoteKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final KeyPair vrfKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final MessageEncoder encoder = new MessageEncoder(keyPair);
			final byte[] encoded = encoder.encodePersistentHarvestingDelegation(nodeKeyPair.getPublicKey(), remoteKeyPair, vrfKeyPair);

			// - zero public key
			Arrays.fill(encoded, 8, 8 + 32, (byte) 0);

			// Act:
			final MessageEncoder decoder = new MessageEncoder(nodeKeyPair);
			final MessageEncoderResult result = decoder.tryDecode(keyPair.getPublicKey(), encoded);

			// Assert:
			assertThat(result.isDecoded(), is(false));
			assertThat((byte[]) result.message(), is(equalTo(encoded)));
		}
	}

	// endregion

	// region deprecated

	@Nested
	final class Deprecated {
		@Test
		void canCreateEncoder() {
			// Arrange + Act + Assert:
			assertCanCreateEncoder();
		}

		@Test
		void senderCanDecodeEncodedMessage() {
			// Arrange + Act + Assert:
			assertSenderCanDecodeEncodedMessage(MessageEncoder::encodeDeprecated, MessageEncoder::tryDecodeDeprecated);
		}

		@Test
		void recipientCanDecodeEncodedMessage() {
			// Arrange + Act + Assert:
			assertRecipientCanDecodeEncodedMessage(MessageEncoder::encodeDeprecated, MessageEncoder::tryDecodeDeprecated);
		}

		@Test
		void decodeFallsBackToInputWhenDecodingFailedShort() {
			// Arrange + Act + Assert:
			assertDecodeFallsBackToInputWhenDecodingFailed(HELLO_WORLD, MessageEncoder::encodeDeprecated,
					MessageEncoder::tryDecodeDeprecated);
		}

		@Test
		void decodeFallsBackToInputWhenDecodingFailedLong() {
			// Arrange + Act + Assert:
			assertDecodeFallsBackToInputWhenDecodingFailed(LONG_MESSAGE, MessageEncoder::encodeDeprecated,
					MessageEncoder::tryDecodeDeprecated);
		}

		@Test
		void fallsBackToDecodeOnFailure() {
			// Arrange: encode using non-deprecated function
			final KeyPair keyPair = new KeyPair(CryptoTypes.PrivateKey.random());
			final CryptoTypes.PublicKey recipientPublicKey = new KeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
			final MessageEncoder encoder = new MessageEncoder(keyPair);
			final byte[] encoded = encoder.encode(recipientPublicKey, HELLO_WORLD);

			// Act: decode using deprecated function
			final MessageEncoderResult result = encoder.tryDecodeDeprecated(recipientPublicKey, encoded);

			// Assert: decode was successful
			assertThat(result.isDecoded(), is(true));
			assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
		}
	}

	// endregion
}
