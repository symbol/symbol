package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.function.Supplier;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.models.*;

/**
 * Tests {@link VotingKeysGenerator}
 */
final class VotingKeysGeneratorTest {
	private static final String ROOT_PRIVATE_KEY_HEX = "0000000000000000000000000000000000000000000000000000000000000001";

	/** Fibonacci private-key generator: 31 zero bytes followed by {@code (a + b) mod 256}. */
	private static Supplier<CryptoTypes.PrivateKey> fibPrivateKeyGenerator() {
		return new Supplier<>() {
			int value1 = 1;
			int value2 = 2;

			@Override
			public CryptoTypes.PrivateKey get() {
				int nextValue = value1 + value2;
				value1 = value2;
				value2 = nextValue;
				byte[] buf = new byte[CryptoTypes.PrivateKey.SIZE];
				buf[CryptoTypes.PrivateKey.SIZE - 1] = (byte) (nextValue % 256);
				return new CryptoTypes.PrivateKey(buf);
			}
		};
	}

	@Nested
	class Generate {
		@Test
		void singleEpochProducesCorrectSize() {
			// Arrange:
			final KeyPair root = new KeyPair(new CryptoTypes.PrivateKey(ROOT_PRIVATE_KEY_HEX));
			final VotingKeysGenerator generator = new VotingKeysGenerator(root, fibPrivateKeyGenerator());

			// Act:
			final byte[] votingKeysBuffer = generator.generate(1L, 1L);

			// Assert:
			assertThat(votingKeysBuffer.length, is(80 + 96));
		}

		@Test
		void rejectsEpochRangeWhoseBufferSizeOverflows() {
			// Arrange: an epoch count fitting an int can still make HEADER + 96 * numEpochs overflow the int buffer size; the guard
			// throws at the size computation (before any signing), rather than allocating an undersized buffer and crashing mid-loop
			final KeyPair root = new KeyPair(new CryptoTypes.PrivateKey(ROOT_PRIVATE_KEY_HEX));
			final VotingKeysGenerator generator = new VotingKeysGenerator(root, fibPrivateKeyGenerator());

			// Act + Assert: 25_000_001 epochs -> 96 * that exceeds Integer.MAX_VALUE
			assertThrows(IllegalArgumentException.class, () -> generator.generate(0L, 25_000_000L));
		}
	}

	@Test
	void canGenerateHeader() {
		// Arrange:
		final KeyPair rootKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final VotingKeysGenerator votingKeysGenerator = new VotingKeysGenerator(rootKeyPair);

		// Act:
		final byte[] votingKeysBuffer = votingKeysGenerator.generate(7L, 11L);

		// Assert:
		final int expectedSize = 32 + CryptoTypes.PublicKey.SIZE + 16 + 5 * (CryptoTypes.PrivateKey.SIZE + CryptoTypes.Signature.SIZE);
		assertThat(votingKeysBuffer.length, is(expectedSize));

		final ByteBuffer reader = ByteBuffer.wrap(votingKeysBuffer).order(ByteOrder.LITTLE_ENDIAN);
		assertThat(reader.getLong(0), is(7L));
		assertThat(reader.getLong(8), is(11L));
		assertThat(reader.getLong(16), is(0xFFFFFFFFFFFFFFFFL));
		assertThat(reader.getLong(24), is(0xFFFFFFFFFFFFFFFFL));

		final byte[] headerRootPublicKey = Arrays.copyOfRange(votingKeysBuffer, 32, 32 + CryptoTypes.PublicKey.SIZE);
		assertThat(headerRootPublicKey, equalTo(rootKeyPair.getPublicKey().bytes()));
		assertThat(reader.getLong(64), is(7L));
		assertThat(reader.getLong(72), is(11L));
	}

	@Test
	void canGenerateRandomChildKeys() {
		// Arrange:
		final KeyPair rootKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final VotingKeysGenerator votingKeysGenerator = new VotingKeysGenerator(rootKeyPair);

		// Act:
		final byte[] votingKeysBuffer = votingKeysGenerator.generate(7L, 11L);

		// Assert:
		final int expectedSize = 32 + CryptoTypes.PublicKey.SIZE + 16 + 5 * (CryptoTypes.PrivateKey.SIZE + CryptoTypes.Signature.SIZE);
		assertThat(votingKeysBuffer.length, is(expectedSize));

		final Verifier verifier = new Verifier(rootKeyPair.getPublicKey());
		for (int i = 0; i < 5; ++i) {
			final int startOffset = 80 + 96 * i;
			final byte[] childPrivateKeyBytes = Arrays.copyOfRange(votingKeysBuffer, startOffset,
					startOffset + CryptoTypes.PrivateKey.SIZE);
			final byte[] signatureBytes = Arrays.copyOfRange(votingKeysBuffer, startOffset + CryptoTypes.PrivateKey.SIZE,
					startOffset + CryptoTypes.PrivateKey.SIZE + CryptoTypes.Signature.SIZE);

			final KeyPair childKeyPair = new KeyPair(new CryptoTypes.PrivateKey(childPrivateKeyBytes));
			final ByteBuffer signedPayload = ByteBuffer.allocate(CryptoTypes.PublicKey.SIZE + 8).order(ByteOrder.LITTLE_ENDIAN);
			signedPayload.put(childKeyPair.getPublicKey().bytes());
			signedPayload.putLong(CryptoTypes.PublicKey.SIZE, 11L - i);

			final boolean actual = verifier.verify(signedPayload.array(), new CryptoTypes.Signature(signatureBytes));
			assertThat("child at " + i, actual, is(true));
		}
	}
}
