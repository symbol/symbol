package org.symbol.sdk.test;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

import java.nio.charset.StandardCharsets;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.MessageEncoderResult;

/**
 * Shared decode-failure contract tests for the message encoders.
 *
 * @param <TKeyPair> Concrete key pair type.
 * @param <TEncoder> Concrete message encoder type.
 * @param <TEncoded> Encoded message representation (raw bytes for Symbol, the Message model for NEM).
 */
public abstract class AbstractMessageEncoderDecodeFailureTest<TKeyPair extends KeyPair, TEncoder, TEncoded> {
	protected static final byte[] HELLO_WORLD = "hello world".getBytes(StandardCharsets.UTF_8);

	private static final byte[] LONG_MESSAGE = "bit longer message that should span upon multiple encryption blocks"
			.getBytes(StandardCharsets.UTF_8);

	/**
	 * @param privateKey Private key.
	 * @return Key pair created around the private key.
	 */
	protected abstract TKeyPair createKeyPair(CryptoTypes.PrivateKey privateKey);

	/**
	 * @param keyPair Key pair.
	 * @return Message encoder created around the key pair.
	 */
	protected abstract TEncoder createEncoder(TKeyPair keyPair);

	/**
	 * @param encoder Message encoder.
	 * @param recipientPublicKey Recipient public key.
	 * @param message Message to encode.
	 * @return Encoded message.
	 */
	protected abstract TEncoded encode(TEncoder encoder, CryptoTypes.PublicKey recipientPublicKey, byte[] message);

	/**
	 * @param encoder Message encoder.
	 * @param publicKey Other party's public key.
	 * @param encoded Encoded message.
	 * @return Decode result.
	 */
	protected abstract MessageEncoderResult tryDecode(TEncoder encoder, CryptoTypes.PublicKey publicKey, TEncoded encoded);

	/**
	 * Malforms an encoded message in place (the variant decides which byte breaks its format).
	 *
	 * @param encoded Encoded message.
	 */
	protected abstract void malformEncoded(TEncoded encoded);

	/**
	 * Projects an encoded message onto its wire bytes.
	 *
	 * @param encoded Encoded message.
	 * @return Wire bytes.
	 */
	protected abstract byte[] encodedToBytes(TEncoded encoded);

	private void runDecodeFailureTest(final byte[] message) {
		// Arrange:
		final TKeyPair keyPair = createKeyPair(CryptoTypes.PrivateKey.random());
		final CryptoTypes.PublicKey recipientPublicKey = createKeyPair(CryptoTypes.PrivateKey.random()).getPublicKey();
		final TEncoder encoder = createEncoder(keyPair);
		final TEncoded encoded = encode(encoder, recipientPublicKey, message);

		malformEncoded(encoded);

		// Act:
		final MessageEncoderResult result = tryDecode(encoder, recipientPublicKey, encoded);

		// Assert: the failed decode falls back to the malformed input
		assertThat(result.isDecoded(), is(false));

		@SuppressWarnings("unchecked")
		final TEncoded fallback = (TEncoded) result.message();
		assertThat(encodedToBytes(fallback), is(equalTo(encodedToBytes(encoded))));
	}

	@Test
	void decodeFallsBackToInputWhenDecodingFailedShort() {
		runDecodeFailureTest(HELLO_WORLD);
	}

	@Test
	void decodeFallsBackToInputWhenDecodingFailedLong() {
		runDecodeFailureTest(LONG_MESSAGE);
	}
}
