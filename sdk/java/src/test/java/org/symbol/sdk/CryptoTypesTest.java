package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.security.SecureRandom;
import java.util.function.Function;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class CryptoTypesTest {
	<T extends ByteArray> void assertCannotCreateFromInvalidLength(Function<byte[], T> factory, final int invalidLength) {
		assertThrows(IllegalArgumentException.class, () -> factory.apply(new byte[invalidLength]));
	}

	private static <T extends ByteArray> void assertHexRoundTrips(final Function<String, T> fromHex, final Function<byte[], T> fromBytes,
			final String hex, final int expectedSize) {
		// Act:
		final T value = fromHex.apply(hex);

		// Assert:
		assertThat(value.toString(), equalTo(hex));
		assertThat(value.bytes().length, equalTo(expectedSize));
		assertThat(fromBytes.apply(value.bytes()), equalTo(value));
	}

	@Nested
	final class Hash256Test {
		@Test
		void hasCorrectSize() {
			assertThat(CryptoTypes.Hash256.SIZE, equalTo(32));
		}

		@Test
		void canCreateZero() {
			// Act:
			final CryptoTypes.Hash256 zero = CryptoTypes.Hash256.zero();

			// Assert:
			assertThat(zero.bytes().length, equalTo(32));
			for (byte b : zero.bytes())
				assertThat(b, equalTo((byte) 0));
		}

		@Test
		void cannotCreateFromInvalidLength() {
			assertCannotCreateFromInvalidLength(CryptoTypes.Hash256::new, 0);
			assertCannotCreateFromInvalidLength(CryptoTypes.Hash256::new, 31);
			assertCannotCreateFromInvalidLength(CryptoTypes.Hash256::new, 33);
		}

		@Test
		void canCreateFromHexString() {
			assertHexRoundTrips(CryptoTypes.Hash256::new, CryptoTypes.Hash256::new,
					"36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8", 32);
		}
	}

	@Nested
	final class PrivateKeyTest {
		@Test
		void hasCorrectSize() {
			assertThat(CryptoTypes.PrivateKey.SIZE, equalTo(32));
		}

		@Test
		void canCreateRandom() {
			// Act:
			final CryptoTypes.PrivateKey k1 = CryptoTypes.PrivateKey.random();
			final CryptoTypes.PrivateKey k2 = CryptoTypes.PrivateKey.random();

			// Assert: two independent draws differ (random() never returns null, so a not-null check would be vacuous)
			assertThat(k1.equals(k2), equalTo(false));
		}

		@Test
		void cannotCreateFromInvalidLength() {
			assertCannotCreateFromInvalidLength(CryptoTypes.PrivateKey::new, 0);
			assertCannotCreateFromInvalidLength(CryptoTypes.PrivateKey::new, 31);
			assertCannotCreateFromInvalidLength(CryptoTypes.PrivateKey::new, 33);
		}

		@Test
		void canCreateFromBytesAndHexString() {
			assertHexRoundTrips(CryptoTypes.PrivateKey::new, CryptoTypes.PrivateKey::new,
					"3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B", 32);
		}
	}

	@Nested
	final class PublicKeyTest {
		private static final String HEX = "1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A";

		@Test
		void hasCorrectSize() {
			assertThat(CryptoTypes.PublicKey.SIZE, equalTo(32));
		}

		@Test
		void copyConstructorPreservesBytes() {
			// Arrange:
			final SecureRandom random = new SecureRandom();
			final byte[] bytes = new byte[32];
			random.nextBytes(bytes);
			final CryptoTypes.PublicKey original = new CryptoTypes.PublicKey(bytes);

			// Act:
			final CryptoTypes.PublicKey copy = new CryptoTypes.PublicKey(original);

			// Assert:
			assertThat(copy.bytes(), equalTo(original.bytes()));
		}

		@Test
		void canCreateFromHexString() {
			assertHexRoundTrips(CryptoTypes.PublicKey::new, CryptoTypes.PublicKey::new, HEX, 32);
		}

		@Test
		void cannotCreateFromInvalidLength() {
			assertCannotCreateFromInvalidLength(CryptoTypes.PublicKey::new, 0);
			assertCannotCreateFromInvalidLength(CryptoTypes.PublicKey::new, 31);
			assertCannotCreateFromInvalidLength(CryptoTypes.PublicKey::new, 33);
		}
	}

	@Nested
	final class SharedKey256Test {
		@Test
		void hasCorrectSize() {
			assertThat(CryptoTypes.SharedKey256.SIZE, equalTo(32));
		}

		@Test
		void canCreateFromBytesAndHexString() {
			assertHexRoundTrips(CryptoTypes.SharedKey256::new, CryptoTypes.SharedKey256::new,
					"7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97", 32);
		}

		@Test
		void cannotCreateFromInvalidLength() {
			assertCannotCreateFromInvalidLength(CryptoTypes.SharedKey256::new, 0);
			assertCannotCreateFromInvalidLength(CryptoTypes.SharedKey256::new, 31);
			assertCannotCreateFromInvalidLength(CryptoTypes.SharedKey256::new, 33);
		}
	}

	@Nested
	final class SignatureTest {
		@Test
		void hasCorrectSize() {
			assertThat(CryptoTypes.Signature.SIZE, equalTo(64));
		}

		@Test
		void cannotCreateFromInvalidLength() {
			assertCannotCreateFromInvalidLength(CryptoTypes.Signature::new, 0);
			assertCannotCreateFromInvalidLength(CryptoTypes.Signature::new, 63);
			assertCannotCreateFromInvalidLength(CryptoTypes.Signature::new, 65);
		}

		@Test
		void canCreateZero() {
			// Act:
			final CryptoTypes.Signature zero = CryptoTypes.Signature.zero();

			// Assert:
			assertThat(zero.bytes().length, equalTo(64));
			assertThat(zero.toString(), equalTo("0".repeat(128)));
		}

		@Test
		void canCreateFromHexString() {
			assertHexRoundTrips(CryptoTypes.Signature::new, CryptoTypes.Signature::new, "F".repeat(128), 64);
		}
	}
}
