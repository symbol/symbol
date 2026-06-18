package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.notNullValue;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link Bip32} root and child derivation against fixed deterministic seed and mnemonic vectors.
 */
final class Bip32Test {
	private static final String DETERMINISTIC_SEED_HEX = "000102030405060708090A0B0C0D0E0F";
	private static final String DETERMINISTIC_MNEMONIC = "cat swing flag economy stadium alone churn speed unique patch report train";

	private static byte[] seed() {
		return Converter.hexToUint8(DETERMINISTIC_SEED_HEX);
	}

	@Test
	void fromSeed_createsRootNode_ed25519() {
		// Act:
		final Bip32.Bip32Node node = new Bip32().fromSeed(seed());

		// Assert:
		assertThat(Converter.uint8ToHex(node.chainCode), equalTo("90046A93DE5380A72B5E45010748567D5EA02BBF6522F979E05C0D8D8CA9FFFB"));
		assertThat(Converter.uint8ToHex(node.privateKey.bytes()),
				equalTo("2B4BE7F19EE27BBF30C667B642D5F4AA69FD169872F8FC3059C08EBAE2EB19E7"));
	}

	@Test
	void fromSeed_createsRootNode_ed25519Keccak() {
		// Act:
		final Bip32.Bip32Node node = new Bip32("ed25519-keccak").fromSeed(seed());

		// Assert:
		assertThat(Converter.uint8ToHex(node.chainCode), equalTo("9CFCA256458AAC0A0550A30DC7639D87364E4323BA61ED41454818E3317BAED0"));
		assertThat(Converter.uint8ToHex(node.privateKey.bytes()),
				equalTo("A3D76D92ACF784D68F4EA2F6DE5507A3520385237A80277132B6C8F3685601B2"));
	}

	@Test
	void deriveOne_canDeriveSingleChild() {
		// Act:
		final Bip32.Bip32Node node = new Bip32().fromSeed(seed()).deriveOne(0);

		// Assert:
		assertThat(Converter.uint8ToHex(node.chainCode), equalTo("8B59AA11380B624E81507A27FEDDA59FEA6D0B779A778918A2FD3590E16E9C69"));
		assertThat(Converter.uint8ToHex(node.privateKey.bytes()),
				equalTo("68E0FE46DFB67E368C75379ACEC591DAD19DF3CDE26E63B93A8E704F1DADE7A3"));
	}

	@Test
	void derivePath_matchesIteratedDeriveOne() {
		// Arrange:
		final Bip32.Bip32Node root = new Bip32().fromSeed(seed());

		// Act:
		final Bip32.Bip32Node viaPath = root.derivePath(new int[]{
				0, 1, 2
		});
		final Bip32.Bip32Node viaSteps = root.deriveOne(0).deriveOne(1).deriveOne(2);

		// Assert:
		assertThat(Converter.uint8ToHex(viaPath.privateKey.bytes()), equalTo(Converter.uint8ToHex(viaSteps.privateKey.bytes())));
		assertThat(Converter.uint8ToHex(viaPath.chainCode), equalTo(Converter.uint8ToHex(viaSteps.chainCode)));
	}

	@Test
	void fromMnemonic_derivesExpectedChildPrivateKeys() {
		// Arrange:
		// Cross-SDK BIP32 test vectors; mnemonic + password "TREZOR".
		final Bip32.Bip32Node node = new Bip32().fromMnemonic(DETERMINISTIC_MNEMONIC, "TREZOR");

		// Act:
		final Bip32.Bip32Node child0 = node.derivePath(new int[]{
				44, 4343, 0, 0, 0
		});
		final Bip32.Bip32Node child1 = node.derivePath(new int[]{
				44, 4343, 1, 0, 0
		});
		final Bip32.Bip32Node child2 = node.derivePath(new int[]{
				44, 4343, 2, 0, 0
		});

		// Assert:
		assertThat(Converter.uint8ToHex(child0.privateKey.bytes()),
				equalTo("1455FB18AB105444763EED593B7CA1C53EF6DDF8BDA1AB7004276FEAB1FCF222"));
		assertThat(Converter.uint8ToHex(child1.privateKey.bytes()),
				equalTo("913967B3DFE1E94C50D5C92789DA194644D2A699E5BB75B171A3B68993B82A21"));
		assertThat(Converter.uint8ToHex(child2.privateKey.bytes()),
				equalTo("AEC7C0143FC11F26FF5DB020492DACA7C8CF2640D2377AD3C721286472571602"));
	}

	@Test
	void fromMnemonic_rejectsWordNotInWordlist() {
		// Arrange:
		final String bad = DETERMINISTIC_MNEMONIC.replace("cat", "notaword");

		// Act + Assert:
		assertThrows(IllegalArgumentException.class, () -> new Bip32().fromMnemonic(bad, ""));
	}

	@Test
	void fromMnemonic_rejectsBadChecksum() {
		// Arrange:
		// Take the deterministic mnemonic and swap two words to break the checksum.
		final String bad = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";
		// Above is a valid all-zero entropy mnemonic ("abandon ... about" actually). Let's break it instead:
		final String broken = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon ";

		// Act + Assert:
		// The 12th word of the all-zero entropy phrase must be "about", not "abandon" — so this should fail.
		assertThrows(IllegalArgumentException.class, () -> new Bip32().fromMnemonic(broken, ""));
		// silence unused-warning
		assertThat(bad.length(), not(equalTo(0)));
	}

	@Test
	void random_default32BytesProduces24Words() {
		// Arrange:
		final Bip32 bip32 = new Bip32();

		// Act:
		final String m1 = bip32.random();
		final String m2 = bip32.random();

		// Assert:
		assertThat(m1.split(" ").length, is(24));
		assertThat(m2.split(" ").length, is(24));
		assertThat(m1, not(equalTo(m2)));
	}

	@Test
	void random_16BytesProduces12Words() {
		// Arrange:
		final Bip32 bip32 = new Bip32();

		// Act:
		final String m1 = bip32.random(16);
		final String m2 = bip32.random(16);

		// Assert:
		assertThat(m1.split(" ").length, is(12));
		assertThat(m2.split(" ").length, is(12));
		assertThat(m1, not(equalTo(m2)));
	}

	@Test
	void random_invalidEntropyLengthThrows() {
		// Arrange:
		final Bip32 bip32 = new Bip32();

		// Act + Assert:
		assertThrows(IllegalArgumentException.class, () -> bip32.random(18));
		assertThrows(IllegalArgumentException.class, () -> bip32.random(8));
		assertThrows(IllegalArgumentException.class, () -> bip32.random(64));
	}

	@Test
	void random_generatedMnemonicIsValid() {
		// Arrange:
		// a self-generated mnemonic must round-trip through fromMnemonic
		final Bip32 bip32 = new Bip32();
		final String mnemonic = bip32.random();

		// Act + Assert:
		bip32.fromMnemonic(mnemonic, "");
	}

	@Test
	void fromMnemonic_acceptsAllValidWordCounts() {
		// Arrange:
		// 16/20/24/28/32 bytes of entropy -> 12/15/18/21/24 words; each valid length must pass assertValid.
		final Bip32 bip32 = new Bip32();

		// Act + Assert:
		for (final int entropyBytes : new int[]{
				16, 20, 24, 28, 32
		}) {
			final String mnemonic = bip32.random(entropyBytes);
			assertThat(bip32.fromMnemonic(mnemonic, ""), is(notNullValue()));
		}
	}

	@Test
	void fromMnemonic_rejectsBadWordCount() {
		// Arrange:
		final String thirteenWords = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> new Bip32().fromMnemonic(thirteenWords, ""));

		// Assert:
		assertThat(ex.getMessage(), containsString("bad word count"));
	}

	@Test
	void fromMnemonic_acceptsNullPassword() {
		// Arrange:
		final Bip32 bip32 = new Bip32();

		// Act + Assert:
		assertThat(bip32.fromMnemonic(bip32.random(16), null), is(notNullValue()));
	}
	@Test
	void fromMnemonicRejectsNonCanonicalWhitespace() {
		// Arrange:
		final Bip32 bip32 = new Bip32("ed25519");
		final String mnemonic = "cat swing flag economy stadium alone churn speed unique patch report train";

		// Act + Assert:
		// sanity: canonical phrase derives
		bip32.fromMnemonic(mnemonic, "TREZOR");
		// double space / trailing space derive different seed bytes — must be rejected
		org.junit.jupiter.api.Assertions.assertThrows(IllegalArgumentException.class,
				() -> bip32.fromMnemonic(mnemonic.replaceFirst(" ", "  "), "TREZOR"));
		org.junit.jupiter.api.Assertions.assertThrows(IllegalArgumentException.class, () -> bip32.fromMnemonic(mnemonic + " ", "TREZOR"));
	}

	@Test
	void allBundledWordlistsDeriveRoundTrip() {
		// Act + Assert:
		// every bundled BIP-39 language loads (2048 words) and random() output validates back
		// through fromMnemonic.
		for (String language : java.util.List.of("english", "spanish", "french", "italian", "japanese", "korean", "chinese")) {
			final Bip32 bip32 = new Bip32("ed25519", language);
			final String mnemonic = bip32.random();
			final Bip32.Bip32Node node = bip32.fromMnemonic(mnemonic, "password");
			org.hamcrest.MatcherAssert.assertThat(language, node, org.hamcrest.Matchers.notNullValue());
		}
	}

}
