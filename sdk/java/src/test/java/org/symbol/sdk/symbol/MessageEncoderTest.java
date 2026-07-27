package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;
import org.symbol.sdk.test.AbstractMessageEncoderDecodeFailureTest;
import org.symbol.sdk.test.AbstractMessageEncoderTest;

/**
 * Tests {@link MessageEncoder}: the shared encoder contract runs via {@link AbstractMessageEncoderTest} per variant plus the delegation and
 * deprecated wallet flows.
 */
final class MessageEncoderTest {

	private static void malformLastByte(final byte[] encoded) {
		encoded[encoded.length - 1] ^= (byte) 0xFF;
	}

	// simulates a delegation message where node and ephemeral key pairs are used; for the delegation failure tests to work properly,
	// the encoder key pair is used as the node key pair (the recipient public key and message arguments are ignored, like in JS)
	private static byte[] encodeDelegation(final MessageEncoder encoder) {
		final KeyPair remoteKeyPair = new KeyPair(
				new CryptoTypes.PrivateKey("11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF"));
		final KeyPair vrfKeyPair = new KeyPair(
				new CryptoTypes.PrivateKey("11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF"));
		return encoder.encodePersistentHarvestingDelegation(encoder.getPublicKey(), remoteKeyPair, vrfKeyPair);
	}

	private abstract class BasicMessageEncoderTest extends AbstractMessageEncoderTest<KeyPair, MessageEncoder, byte[]> {
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
		protected byte[] encodedToBytes(final byte[] encoded) {
			return encoded;
		}

		@Override
		protected void malformEncoded(final byte[] encoded) {
			malformLastByte(encoded);
		}
	}

	// region recommended

	@Nested
	final class Recommended extends BasicMessageEncoderTest {
		@Override
		protected byte[] encode(final MessageEncoder encoder, final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
			return encoder.encode(recipientPublicKey, message);
		}

		@Override
		protected MessageEncoderResult tryDecode(final MessageEncoder encoder, final CryptoTypes.PublicKey publicKey,
				final byte[] encoded) {
			return encoder.tryDecode(publicKey, encoded);
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
	final class Delegation extends AbstractMessageEncoderDecodeFailureTest<KeyPair, MessageEncoder, byte[]> {
		@Override
		protected KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
			return new KeyPair(privateKey);
		}

		@Override
		protected MessageEncoder createEncoder(final KeyPair keyPair) {
			return new MessageEncoder(keyPair);
		}

		@Override
		protected byte[] encode(final MessageEncoder encoder, final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
			return encodeDelegation(encoder);
		}

		@Override
		protected MessageEncoderResult tryDecode(final MessageEncoder encoder, final CryptoTypes.PublicKey publicKey,
				final byte[] encoded) {
			return encoder.tryDecode(publicKey, encoded);
		}

		@Override
		protected byte[] encodedToBytes(final byte[] encoded) {
			return encoded;
		}

		@Override
		protected void malformEncoded(final byte[] encoded) {
			malformLastByte(encoded);
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
	final class Deprecated extends BasicMessageEncoderTest {
		@Override
		protected byte[] encode(final MessageEncoder encoder, final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
			return encoder.encodeDeprecated(recipientPublicKey, message);
		}

		@Override
		protected MessageEncoderResult tryDecode(final MessageEncoder encoder, final CryptoTypes.PublicKey publicKey,
				final byte[] encoded) {
			return encoder.tryDecodeDeprecated(publicKey, encoded);
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
