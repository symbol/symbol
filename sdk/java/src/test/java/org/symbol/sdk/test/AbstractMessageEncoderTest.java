package org.symbol.sdk.test;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.MessageEncoderResult;

/**
 * Shared message encoder contract tests (decode successes plus the inherited decode failures)
 *
 * @param <TKeyPair> Concrete key pair type.
 * @param <TEncoder> Concrete message encoder type.
 * @param <TEncoded> Encoded message representation (raw bytes for Symbol, the Message model for NEM).
 */
public abstract class AbstractMessageEncoderTest<TKeyPair extends KeyPair, TEncoder, TEncoded>
		extends
			AbstractMessageEncoderDecodeFailureTest<TKeyPair, TEncoder, TEncoded> {
	/**
	 * @param encoder Message encoder.
	 * @return Public key used for message encoding.
	 */
	protected abstract CryptoTypes.PublicKey publicKeyOf(TEncoder encoder);

	@Test
	void canCreateEncoder() {
		// Arrange:
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());

		// Act:
		final TEncoder encoder = createEncoder(keyPair);

		// Assert:
		assertThat(publicKeyOf(encoder), is(equalTo(keyPair.getPublicKey())));
	}

	private void assertDecodeSuccess(final TEncoder decoder, final CryptoTypes.PublicKey otherPublicKey, final TEncoded encoded) {
		// Act:
		final MessageEncoderResult result = tryDecode(decoder, otherPublicKey, encoded);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat((byte[]) result.message(), is(equalTo(HELLO_WORLD)));
	}

	/** Random sender and recipient key pairs with a message encoded from the sender to the recipient. */
	private final class EncodedMessageContext {
		private final TKeyPair senderKeyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		private final TKeyPair recipientKeyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		private final TEncoder senderEncoder = createEncoder(senderKeyPair);
		private final TEncoded encoded = encode(senderEncoder, recipientKeyPair.getPublicKey(), HELLO_WORLD);
	}

	@Test
	void senderCanDecodeEncodedMessage() {
		// Arrange:
		final EncodedMessageContext context = new EncodedMessageContext();

		// the sender decodes with its own encoder using the recipient's public key
		assertDecodeSuccess(context.senderEncoder, context.recipientKeyPair.getPublicKey(), context.encoded);
	}

	@Test
	void recipientCanDecodeEncodedMessage() {
		// Arrange:
		final EncodedMessageContext context = new EncodedMessageContext();

		// the recipient decodes with its own encoder using the sender's public key
		final TEncoder decoder = createEncoder(context.recipientKeyPair);
		assertDecodeSuccess(decoder, context.senderKeyPair.getPublicKey(), context.encoded);
	}
}
