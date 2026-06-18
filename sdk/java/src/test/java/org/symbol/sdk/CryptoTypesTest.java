package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.notNullValue;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.security.SecureRandom;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class CryptoTypesTest {
	@Nested
	final class Hash256Test {
		@Test
		void hasCorrectSize() {
			// Act + Assert:
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
			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> new CryptoTypes.Hash256(new byte[31]));
		}

		@Test
		void canCreateFromHexString() {
			// Arrange:
			final String hex = "36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8";

			// Act:
			final CryptoTypes.Hash256 hash = new CryptoTypes.Hash256(hex);

			// Assert:
			assertThat(hash.bytes().length, equalTo(32));
			assertThat(hash.toString(), equalTo(hex));
		}

		@Test
		void parseAcceptsTypedHexStringAndRawBytes() {
			// Arrange:
			final String hex = "36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8";
			final CryptoTypes.Hash256 typed = new CryptoTypes.Hash256(hex);

			// Act + Assert:
			assertThat(CryptoTypes.Hash256.parse(typed), sameInstance(typed)); // existing value passed through
			assertThat(CryptoTypes.Hash256.parse(hex), equalTo(typed)); // hex string
			assertThat(CryptoTypes.Hash256.parse(typed.bytes()), equalTo(typed)); // raw byte[]
		}
	}

	@Nested
	final class PrivateKeyTest {
		@Test
		void hasCorrectSize() {
			// Act + Assert:
			assertThat(CryptoTypes.PrivateKey.SIZE, equalTo(32));
		}

		@Test
		void canCreateRandom() {
			// Act:
			final CryptoTypes.PrivateKey k1 = CryptoTypes.PrivateKey.random();
			final CryptoTypes.PrivateKey k2 = CryptoTypes.PrivateKey.random();

			// Assert:
			assertThat(k1, notNullValue());
			assertThat(k2, notNullValue());
			assertThat(k1.equals(k2), equalTo(false));
		}

		@Test
		void canCreateFromBytesAndHexString() {
			// Arrange:
			final String hex = "3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B2B3C2B";

			// Act:
			final CryptoTypes.PrivateKey fromHex = new CryptoTypes.PrivateKey(hex);

			// Assert:
			assertThat(fromHex.toString(), equalTo(hex));
			assertThat(new CryptoTypes.PrivateKey(fromHex.bytes()), equalTo(fromHex));
		}
	}

	@Nested
	final class PublicKeyTest {
		private static final String HEX = "1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A1A";

		@Test
		void hasCorrectSize() {
			// Act + Assert:
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
			// Act:
			final CryptoTypes.PublicKey key = new CryptoTypes.PublicKey(HEX);

			// Assert:
			assertThat(key.bytes().length, equalTo(32));
			assertThat(key.toString(), equalTo(HEX));
		}

		@Test
		void parseAcceptsTypedHexStringAndRawBytes() {
			// Arrange:
			final CryptoTypes.PublicKey typed = new CryptoTypes.PublicKey(HEX);

			// Act + Assert:
			assertThat(CryptoTypes.PublicKey.parse(typed), sameInstance(typed)); // existing value passed through
			assertThat(CryptoTypes.PublicKey.parse(HEX), equalTo(typed)); // hex string
			assertThat(CryptoTypes.PublicKey.parse(typed.bytes()), equalTo(typed)); // raw byte[]
		}
	}

	@Nested
	final class SharedKey256Test {
		@Test
		void hasCorrectSize() {
			// Act + Assert:
			assertThat(CryptoTypes.SharedKey256.SIZE, equalTo(32));
		}

		@Test
		void canCreateFromBytesAndHexString() {
			// Arrange:
			final String hex = "7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97";

			// Act:
			final CryptoTypes.SharedKey256 key = new CryptoTypes.SharedKey256(hex);

			// Assert:
			assertThat(key.toString(), equalTo(hex));
			assertThat(new CryptoTypes.SharedKey256(key.bytes()), equalTo(key));
		}
	}

	@Nested
	final class SignatureTest {
		@Test
		void hasCorrectSize() {
			// Act + Assert:
			assertThat(CryptoTypes.Signature.SIZE, equalTo(64));
		}

		@Test
		void canCreateZero() {
			// Act:
			final CryptoTypes.Signature zero = CryptoTypes.Signature.zero();

			// Assert:
			assertThat(zero.bytes().length, equalTo(64));
		}

		@Test
		void canCreateFromHexString() {
			// Arrange:
			final String hex = "F".repeat(128);

			// Act:
			final CryptoTypes.Signature sig = new CryptoTypes.Signature(hex);

			// Assert:
			assertThat(sig.bytes().length, equalTo(64));
			assertThat(sig.toString(), equalTo(hex));
		}
	}
}
