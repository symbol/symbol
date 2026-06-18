package org.symbol.sdk.impl;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.utils.Converter;

/**
 * Tests the {@link Ed25519} ed25519-with-swappable-hash implementation: the SHA-512 path against RFC 8032 §7.1 vectors, the Keccak-512 path
 * against a deterministic NEM keypair vector.
 */
final class Ed25519Test {
	@Test
	void sha512_keypairFromSeed_matchesSymbolDeterministicVector() {
		// Arrange:
		// Cross-SDK deterministic keypair test vector
		final byte[] seed = Converter.hexToUint8("E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E");

		// Act:
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);

		// Assert:
		assertThat(Converter.uint8ToHex(kp.publicKey), equalTo("E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD"));
	}

	@Test
	void sha512_signAndVerifyRoundtrip() {
		// Arrange:
		final byte[] seed = Converter.hexToUint8("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		final byte[] msg = "Hello, Symbol!".getBytes();

		// Act:
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);

		// Assert:
		assertThat(sig.length, is(64));
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, msg, sig, kp.publicKey), is(true));
	}

	@Test
	void sha512_rfc8032_test1() {
		// Arrange:
		// RFC 8032 §7.1 Test 1
		final byte[] seed = Converter.hexToUint8("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
		final String expectedPublicKey = "D75A980182B10AB7D54BFED3C964073A0EE172F3DAA62325AF021A68F707511A";
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		assertThat(Converter.uint8ToHex(kp.publicKey), equalTo(expectedPublicKey));

		// Act:
		final byte[] msg = new byte[0];
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);

		// Assert:
		final String expectedSignature = "E5564300C360AC729086E2CC806E828A84877F1EB8E5D974D873E06522490155"
				+ "5FB8821590A33BACC61E39701CF9B46BD25BF5F0595BBE24655141438E7A100B";
		assertThat(Converter.uint8ToHex(sig), equalTo(expectedSignature));
	}

	@Test
	void sha512_rfc8032_test2() {
		// Arrange:
		// RFC 8032 §7.1 Test 2
		final byte[] seed = Converter.hexToUint8("4CCD089B28FF96DA9DB6C346EC114E0F5B8A319F35ABA624DA8CF6ED4FB8A6FB");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		assertThat(Converter.uint8ToHex(kp.publicKey), equalTo("3D4017C3E843895A92B70AA74D1B7EBC9C982CCF2EC4968CC0CD55F12AF4660C"));

		// Act:
		final byte[] msg = Converter.hexToUint8("72");
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);

		// Assert:
		final String expectedSignature = "92A009A9F0D4CAB8720E820B5F642540A2B27B5416503F8FB3762223EBDB69DA"
				+ "085AC1E43E15996E458F3613D0F11D8C387B2EAEB4302AEEB00D291612BB0C00";
		assertThat(Converter.uint8ToHex(sig), equalTo(expectedSignature));
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, msg, sig, kp.publicKey), is(true));
	}

	@Test
	void sha512_verifyFailsOnTamperedMessage() {
		// Arrange:
		final byte[] seed = Converter.hexToUint8("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		final byte[] msg = "Hello".getBytes();
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);
		final byte[] tampered = "Hellp".getBytes();

		// Act + Assert:
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, tampered, sig, kp.publicKey), is(false));
	}

	@Test
	void sha512_verifyFailsOnTamperedSignature() {
		// Arrange:
		final byte[] seed = Converter.hexToUint8("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		final byte[] msg = "Hello".getBytes();
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);

		// Act:
		sig[0] ^= 1;

		// Assert:
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, msg, sig, kp.publicKey), is(false));
	}

	@Test
	void keccak512_signAndVerifyRoundtrip() {
		// Arrange:
		final byte[] seed = Converter.hexToUint8("E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.KECCAK_512, seed);
		final byte[] msg = "NEM message".getBytes();

		// Act:
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.KECCAK_512, msg, kp);

		// Assert:
		assertThat(sig.length, is(64));
		assertThat(Ed25519.verify(Tweetnacl.HashMode.KECCAK_512, msg, sig, kp.publicKey), is(true));
	}

	@Test
	void keccak512_signaturesAreModeSpecific() {
		// Arrange:
		// A signature produced with KECCAK_512 must not verify under SHA2_512 and vice versa.
		final byte[] seed = Converter.hexToUint8("E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E");
		final byte[] msg = "cross-mode".getBytes();

		final Tweetnacl.KeyPair kpKeccak = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.KECCAK_512, seed);
		final byte[] sigKeccak = Ed25519.sign(Tweetnacl.HashMode.KECCAK_512, msg, kpKeccak);

		final Tweetnacl.KeyPair kpSha = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);

		// Act + Assert:
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, msg, sigKeccak, kpSha.publicKey), is(false));
	}

	@Test
	void rejectsNonCanonicalS() {
		// Arrange:
		// An "all 0xFF" S component must be rejected as non-canonical.
		final byte[] seed = Converter.hexToUint8("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		final byte[] msg = "msg".getBytes();
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);

		// Act:
		// Replace S (last 32 bytes) with all 0xFF — this is > L so non-canonical.
		for (int i = 32; i < 64; ++i)
			sig[i] = (byte) 0xFF;

		// Assert:
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, msg, sig, kp.publicKey), is(false));
	}

	// ed25519 group order L, little-endian (2^252 + 27742317777372353535851937790883648493).
	private static final byte[] GROUP_ORDER_L = {
			(byte) 0xED, (byte) 0xD3, (byte) 0xF5, 0x5C, 0x1A, 0x63, 0x12, 0x58, (byte) 0xD6, (byte) 0x9C, (byte) 0xF7, (byte) 0xA2,
			(byte) 0xDE, (byte) 0xF9, (byte) 0xDE, 0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10
	};

	@Test
	void rejectsMalleableSignatureWhereSEqualsCanonicalPlusL() {
		// Arrange:
		// A signature (R, S+L) still satisfies the curve equation (because [L]B is the identity), so the raw
		// curve verify passes -- but S+L >= L is non-canonical, so Ed25519.verify must reject it via isCanonicalS.
		final byte[] seed = Converter.hexToUint8("9D61B19DEFFD5A60BA844AF492EC2CC44449C5697B326919703BAC031CAE7F60");
		final Tweetnacl.KeyPair kp = Ed25519.keyPairFromSeed(Tweetnacl.HashMode.SHA2_512, seed);
		final byte[] msg = "malleable".getBytes();
		final byte[] sig = Ed25519.sign(Tweetnacl.HashMode.SHA2_512, msg, kp);

		// Act:
		final byte[] malleable = sig.clone();
		int carry = 0;
		for (int i = 0; i < 32; ++i) {
			final int sum = (malleable[32 + i] & 0xFF) + (GROUP_ORDER_L[i] & 0xFF) + carry;
			malleable[32 + i] = (byte) sum;
			carry = sum >>> 8;
		}

		// Assert:
		assertThat(Tweetnacl.signDetachedVerify(msg, malleable, kp.publicKey, Tweetnacl.HashMode.SHA2_512), is(true));
		assertThat(Ed25519.verify(Tweetnacl.HashMode.SHA2_512, msg, malleable, kp.publicKey), is(false));
	}
}
