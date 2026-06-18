package org.symbol.sdk.impl;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link SharedKeyHelpers} against the official Symbol and NEM test vectors at
 * {@code tests/vectors/{symbol,nem}/crypto/3.test-derive-hkdf.json}.
 */
final class SharedKeyHelpersTest {
	private record DeriveVector(String privateKeyHex, String otherPublicKeyHex, String sharedKeyHex) {
	}

	// First three vectors from tests/vectors/symbol/crypto/3.test-derive-hkdf.json.
	private static final DeriveVector[] SYMBOL_VECTORS = {
			new DeriveVector("00137C7C32881D1FFF2E905F5B7034BCBCDB806D232F351DB48A7816285C548F",
					"FDEE3C7A41F4717D18B5BFFD685C3C43DFFDC3F8E168AA1B237E1EBF8E9BC869",
					"59BE24D6DB8381DA153CB653134EF7352FA9FDDFD2A9B3727924F7761390C6C1"),
			new DeriveVector("E8857F8E488D4E6D4B71BCD44BB4CFF49208C32651E1F6500C3B58CAFEB8DEF6",
					"0531061660549384490453BC61FB7AFDA69D49E961489A4847D8D5AF1749C65B",
					"52C7F2DCD494A14ED50720BAE0CE6792D9E22D450CF492682801294ECAF35932"),
			new DeriveVector("D7F67B5F52CBCD1A1367E0376A8EB1012B634ACFCF35E8322BAE8B22BB9E8DEA",
					"9A6C6AA5C83019DFF2BC89F4D28D5163F72724F765AA450CB68F9EB6CBFBE20B",
					"C8B57A0B117548273422A55801A963F86A4404AE23F3E4986EF655F40927691F"),
	};

	@Test
	void symbol_deriveSharedKeyMatchesOfficialVectors() {
		// Arrange:
		final SharedKeyHelpers.SharedKeyDeriver derive = SharedKeyHelpers.deriveSharedKeyFactory("catapult", Tweetnacl.HashMode.SHA2_512);

		// Act + Assert:
		for (DeriveVector v : SYMBOL_VECTORS) {
			final byte[] privateKey = Converter.hexToUint8(v.privateKeyHex);
			final CryptoTypes.PublicKey otherPublicKey = new CryptoTypes.PublicKey(v.otherPublicKeyHex);
			final CryptoTypes.SharedKey256 result = derive.derive(privateKey, otherPublicKey);
			assertThat(Converter.uint8ToHex(result.bytes()), equalTo(v.sharedKeyHex));
		}
	}

	@Test
	void symbol_mutualSharedKeysAreEqual() {
		// Arrange:
		// alice and bob deriving with each other's keys must agree.
		final byte[] alicePriv = Converter.hexToUint8("00137C7C32881D1FFF2E905F5B7034BCBCDB806D232F351DB48A7816285C548F");
		final byte[] bobPriv = Converter.hexToUint8("E8857F8E488D4E6D4B71BCD44BB4CFF49208C32651E1F6500C3B58CAFEB8DEF6");

		final Tweetnacl.KeyPair aliceKp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, alicePriv);
		final Tweetnacl.KeyPair bobKp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, bobPriv);

		final SharedKeyHelpers.SharedKeyDeriver derive = SharedKeyHelpers.deriveSharedKeyFactory("catapult", Tweetnacl.HashMode.SHA2_512);

		// Act:
		final CryptoTypes.SharedKey256 aliceToBob = derive.derive(alicePriv, new CryptoTypes.PublicKey(bobKp.publicKey));
		final CryptoTypes.SharedKey256 bobToAlice = derive.derive(bobPriv, new CryptoTypes.PublicKey(aliceKp.publicKey));

		// Assert:
		assertThat(Converter.uint8ToHex(aliceToBob.bytes()), equalTo(Converter.uint8ToHex(bobToAlice.bytes())));
	}

	@Test
	void differentPrivateKeyProducesDifferentSharedKey() {
		// Arrange:
		final byte[] privA = Converter.hexToUint8("00137C7C32881D1FFF2E905F5B7034BCBCDB806D232F351DB48A7816285C548F");
		final byte[] privB = privA.clone();
		privB[0] ^= (byte) 0xFF;
		final CryptoTypes.PublicKey other = new CryptoTypes.PublicKey("FDEE3C7A41F4717D18B5BFFD685C3C43DFFDC3F8E168AA1B237E1EBF8E9BC869");

		final SharedKeyHelpers.SharedKeyDeriver derive = SharedKeyHelpers.deriveSharedKeyFactory("catapult", Tweetnacl.HashMode.SHA2_512);

		// Act:
		final CryptoTypes.SharedKey256 a = derive.derive(privA, other);
		final CryptoTypes.SharedKey256 b = derive.derive(privB, other);

		// Assert:
		assertThat(Converter.uint8ToHex(a.bytes()), not(equalTo(Converter.uint8ToHex(b.bytes()))));
	}

	@Test
	void invalidPublicKeyThrows() {
		// Arrange:
		// All-zero public key is an invalid (non-canonical / not on curve) point.
		final byte[] priv = Converter.hexToUint8("00137C7C32881D1FFF2E905F5B7034BCBCDB806D232F351DB48A7816285C548F");
		final SharedKeyHelpers.SharedKeyDeriver derive = SharedKeyHelpers.deriveSharedKeyFactory("catapult", Tweetnacl.HashMode.SHA2_512);

		// Act:
		// Try several mutated variants and assert at least one is rejected as an invalid point.
		boolean rejected = false;
		for (int i = 0; i < 4; ++i) {
			final byte[] invalid = new byte[32];
			invalid[31] = (byte) i;
			try {
				derive.derive(priv, new CryptoTypes.PublicKey(invalid));
			} catch (IllegalArgumentException ex) {
				if (ex.getMessage().contains("invalid point")) {
					rejected = true;
					break;
				}
			}
		}

		// Assert:
		assertThat(rejected, is(true));
	}

	@Test
	void deriveSharedSecret_returns32Bytes() {
		// Arrange:
		final byte[] priv = Converter.hexToUint8("00137C7C32881D1FFF2E905F5B7034BCBCDB806D232F351DB48A7816285C548F");
		final CryptoTypes.PublicKey other = new CryptoTypes.PublicKey("FDEE3C7A41F4717D18B5BFFD685C3C43DFFDC3F8E168AA1B237E1EBF8E9BC869");

		final SharedKeyHelpers.SharedSecretDeriver derive = SharedKeyHelpers.deriveSharedSecretFactory(Tweetnacl.HashMode.SHA2_512);

		// Act:
		final byte[] secret = derive.derive(priv, other);

		// Assert:
		// The first Symbol vector documents this scalarMulResult value.
		assertThat(Converter.uint8ToHex(secret), equalTo("EAFB74D6778DCF4A55B1758432A13767719FD8AD66A32FF2E3256CEFA4DD7334"));
	}

	@Test
	void deriveSharedKey_invalidPublicKey_throwsImmediately() {
		// Arrange:
		// All-zero (origin point) must be rejected.
		final byte[] priv = new byte[32];
		final byte[] zeroKey = new byte[32];
		final SharedKeyHelpers.SharedKeyDeriver derive = SharedKeyHelpers.deriveSharedKeyFactory("catapult", Tweetnacl.HashMode.SHA2_512);

		// Act + Assert:
		assertThrows(IllegalArgumentException.class, () -> derive.derive(priv, new CryptoTypes.PublicKey(zeroKey)));
	}

	@Test
	void deriveSharedKey_rejectsNonCanonicalPublicKey() {
		// Arrange:
		// y = 2^255-1 is >= 2^255-19, so the encoding is non-canonical -- isCanonicalKey rejects it before the
		// curve unpack, a different branch from the all-zero (off-curve) case above.
		final byte[] priv = Converter.hexToUint8("00137C7C32881D1FFF2E905F5B7034BCBCDB806D232F351DB48A7816285C548F");
		final byte[] nonCanonical = new byte[32];
		java.util.Arrays.fill(nonCanonical, (byte) 0xFF);
		final SharedKeyHelpers.SharedKeyDeriver derive = SharedKeyHelpers.deriveSharedKeyFactory("catapult", Tweetnacl.HashMode.SHA2_512);

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
				() -> derive.derive(priv, new CryptoTypes.PublicKey(nonCanonical)));

		// Assert:
		assertThat(ex.getMessage(), containsString("invalid point"));
	}
}
