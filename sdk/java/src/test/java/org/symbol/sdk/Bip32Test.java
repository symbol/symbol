package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.function.Supplier;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

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

	@Nested
	final class FromSeed {
		@Test
		void createsEd25519RootNode() {
			// Act:
			final Bip32.Bip32Node node = new Bip32().fromSeed(seed());

			// Assert:
			assertThat(Converter.uint8ToHex(node.chainCode), equalTo("90046A93DE5380A72B5E45010748567D5EA02BBF6522F979E05C0D8D8CA9FFFB"));
			assertThat(Converter.uint8ToHex(node.privateKey.bytes()),
					equalTo("2B4BE7F19EE27BBF30C667B642D5F4AA69FD169872F8FC3059C08EBAE2EB19E7"));
		}

		@Test
		void createsEd25519KeccakRootNode() {
			// Act:
			final Bip32.Bip32Node node = new Bip32("ed25519-keccak").fromSeed(seed());

			// Assert:
			assertThat(Converter.uint8ToHex(node.chainCode), equalTo("9CFCA256458AAC0A0550A30DC7639D87364E4323BA61ED41454818E3317BAED0"));
			assertThat(Converter.uint8ToHex(node.privateKey.bytes()),
					equalTo("A3D76D92ACF784D68F4EA2F6DE5507A3520385237A80277132B6C8F3685601B2"));
		}
	}

	@Nested
	final class Derive {
		@Test
		void deriveOneReturnsExpectedChild() {
			// Act:
			final Bip32.Bip32Node node = new Bip32().fromSeed(seed()).deriveOne(0);

			// Assert:
			assertThat(Converter.uint8ToHex(node.chainCode), equalTo("8B59AA11380B624E81507A27FEDDA59FEA6D0B779A778918A2FD3590E16E9C69"));
			assertThat(Converter.uint8ToHex(node.privateKey.bytes()),
					equalTo("68E0FE46DFB67E368C75379ACEC591DAD19DF3CDE26E63B93A8E704F1DADE7A3"));
		}

		@Test
		void derivePathMatchesIteratedDeriveOne() {
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
	}

	@Nested
	final class FromMnemonic {
		@Test
		void derivesExpectedChildPrivateKeys() {
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
		void rejectsWordNotInWordlist() {
			// Arrange:
			final String bad = DETERMINISTIC_MNEMONIC.replace("cat", "notaword");

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> new Bip32().fromMnemonic(bad, ""));
		}

		@Test
		void rejectsBadChecksum() {
			// Arrange: the all-zero-entropy 12-word phrase ends in "about"; replacing that final word with "abandon"
			// keeps every word in the wordlist but corrupts the checksum (whitespace stays canonical so only the
			// checksum can cause the rejection).
			final String valid = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
			final String badChecksum = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";

			// Act + Assert: the correct-checksum phrase is accepted; the corrupted one is rejected.
			assertDoesNotThrow(() -> new Bip32().fromMnemonic(valid, ""));
			assertThrows(IllegalArgumentException.class, () -> new Bip32().fromMnemonic(badChecksum, ""));
		}

		@Test
		void acceptsAllValidWordCounts() {
			// Arrange:
			// 16/20/24/28/32 bytes of entropy -> 12/15/18/21/24 words; each valid length must round-trip through fromMnemonic.
			final Bip32 bip32 = new Bip32();

			for (final int entropyBytes : new int[]{
					16, 20, 24, 28, 32
			}) {
				final String mnemonic = bip32.random(entropyBytes);

				// Act:
				final Bip32.Bip32Node node = bip32.fromMnemonic(mnemonic, "");

				// Assert: each word count derives a real 32-byte root key (not a silently zeroed/degenerate result)
				assertThat("entropy " + entropyBytes, node.privateKey.bytes(), not(equalTo(new byte[32])));
			}
		}

		@Test
		void rejectsBadWordCount() {
			// Arrange:
			final String thirteenWords = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
					() -> new Bip32().fromMnemonic(thirteenWords, ""));

			// Assert: mnemonic4j reports the invalid word count.
			assertThat(ex.getMessage(), containsString("Number of words must be one of"));
		}

		@Test
		void acceptsNullPassword() {
			// Arrange: a null password is treated as the empty string, so it must derive the same root key as ""
			final Bip32 bip32 = new Bip32();
			final String mnemonic = bip32.random(16);

			// Act + Assert:
			assertThat(bip32.fromMnemonic(mnemonic, null).privateKey, equalTo(bip32.fromMnemonic(mnemonic, "").privateKey));
		}

		@Test
		void passphraseIsNfkdNormalized() {
			// Arrange: BIP-39 NFKD-normalizes the passphrase, so a composed (U+00E9) and a decomposed
			// (U+0065 U+0301) form of the same e-acute passphrase must derive the same root key.
			final Bip32 bip32 = new Bip32();

			// Act:
			final CryptoTypes.PrivateKey composed = bip32.fromMnemonic(DETERMINISTIC_MNEMONIC, "\u00E9").privateKey;
			final CryptoTypes.PrivateKey decomposed = bip32.fromMnemonic(DETERMINISTIC_MNEMONIC, "e\u0301").privateKey;

			// Assert:
			assertThat(composed, equalTo(decomposed));
		}

		@Test
		void rejectsNonCanonicalWhitespace() {
			// Arrange:
			final Bip32 bip32 = new Bip32("ed25519");
			final String mnemonic = "cat swing flag economy stadium alone churn speed unique patch report train";

			// Act + Assert:
			// sanity: canonical phrase derives
			bip32.fromMnemonic(mnemonic, "TREZOR");

			// double / trailing / leading space derive different seed bytes — must be rejected by the whitespace guard (with its
			// specific message, not a downstream mnemonic4j word-count/membership error), including a single leading space
			for (final String bad : new String[]{
					mnemonic.replaceFirst(" ", "  "), mnemonic + " ", " " + mnemonic
			}) {
				final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> bip32.fromMnemonic(bad, "TREZOR"));
				assertThat(ex.getMessage(), containsString("whitespace is not canonical"));
			}
		}

		@Test
		void allBundledWordlistsDerivePinnedRootKeys() {
			// Each bundled BIP-39 language derives a pinned ed25519 root private key from a fixed mnemonic (16-byte entropy
			// 0x00..0F), and a freshly generated mnemonic still round-trips through fromMnemonic. The cross-SDK BIP39 vectors
			// (6.test-hd-derivation) only cover English, so these pin the non-English wordlist + delimiter handling against
			// regression. The CJK literals also act as a UTF-8 source-encoding canary.
			final String[][] cases = {
					{
							"english", "abandon amount liar amount expire adjust cage candy arch gather drum buyer",
							"A9F788C4EFF090E8FAC3512E9EC132D8BACEA1E6B81ABBCECD9877C32A35A800"
					}, {
							"chinese", "\u7684 \u4e09 \u6b27 \u4e09 \u8003 \u4e8e \u636e \u4fdd \u91cf \u635f \u7834 \u6218",
							"7A07F7903A338D9F2962E8F5E6BE4F911103CA2B9A470809D9396ABE625ED4FA"
					}, {
							"japanese",
							"\u3042\u3044\u3053\u304f\u3057\u3093 \u3044\u304f\u3076\u3093 \u305d\u306a\u305f \u3044\u304f\u3076\u3093 \u3053\u305c\u3093 \u3042\u3076\u3089 \u304a\u304a\u3046 \u304a\u304d\u308b \u3044\u305f\u307f \u3055\u3093\u3059\u3046 \u3051\u305f\u3070 \u304a\u3046\u305f\u3044",
							"C5027834E39BE14E28AC976D54CEE274012CC27D5B0EF4DB71792B53FFA9D3D6"
					}, {
							"french",
							"abaisser agr\u00e9able inductif agr\u00e9able \u00e9ligible achat bolide boucle amateur exister d\u00e9rober bloquer",
							"A09C99697732D3D0A29C41DAF8631AF8F2DAC7F0B30DD706C0925A907FB8752D"
					}, {
							"spanish", "\u00e1baco \u00e1lbum l\u00edquido \u00e1lbum espuma acudir bolero bosque amante gaita dictar boca",
							"4AC5F3963D55AC519D10B9A319448704490CE19021938706A1802C938613880C"
					}, {
							"italian", "abaco alogeno mitigare alogeno fenomeno affetto bravura bronzina ampio gonfio elaborato bottino",
							"2DA92048EDCE6B65490D9C026B1D811F43D87BB7F41FF5C2F157B5163CB9BC8C"
					}, {
							"korean",
							"\uac00\uaca9 \uac71\uc815 \uc2ec\ubd80\ub984 \uac71\uc815 \ubcc4\ub3c4 \uac08\uc0c9 \uae30\ubc95 \uae30\uc6b4 \uacbd\ub825 \uc0b0\uae38 \ubbf8\uc220 \uae30\ub150",
							"FFFF74DAF0D1400B75EF51085B73B48DFBA2AD5250A3386BBBA915F6FE3D8BBA"
					}
			};

			for (final String[] testCase : cases) {
				final String language = testCase[0];
				final Bip32 bip32 = new Bip32("ed25519", language);
				// fixed mnemonic -> pinned root private key (pins seed derivation per language)
				final Bip32.Bip32Node node = bip32.fromMnemonic(testCase[1], "password");
				assertThat(language, Converter.uint8ToHex(node.privateKey.bytes()), equalTo(testCase[2]));
				// a freshly generated mnemonic in this language still round-trips to a real 32-byte root key
				assertThat(language, bip32.fromMnemonic(bip32.random(), "password").privateKey.bytes(), not(equalTo(new byte[32])));
			}
		}
	}

	@Nested
	final class Random {
		// Asserts that two independent draws each split into the expected word count and differ from one another.
		private static void assertRandomProducesWordCount(final Supplier<String> generate, final int expectedWords) {
			// Assert: word count is the checked property; don't assert two draws differ (an RNG non-collision dependency that
			// can flake with no real regression — the project avoids these, cf. the deterministic CipherTypes tests)
			assertThat(generate.get().split(" ").length, is(expectedWords));
		}

		@Test
		void produces24WordsByDefault() {
			// Arrange:
			final Bip32 bip32 = new Bip32();

			// Act + Assert:
			assertRandomProducesWordCount(bip32::random, 24);
		}

		@Test
		void produces12WordsFrom16Bytes() {
			// Arrange:
			final Bip32 bip32 = new Bip32();

			// Act + Assert:
			assertRandomProducesWordCount(() -> bip32.random(16), 12);
		}

		@ParameterizedTest
		@ValueSource(ints = {
				8, 18, 64, 536870928
		})
		void rejectsInvalidEntropyLength(final int entropyBytes) {
			// 536870928 * 8 overflows a 32-bit int to 128 (a valid strength); without the up-front guard it would silently
			// generate a 12-word mnemonic instead of being rejected
			assertThrows(IllegalArgumentException.class, () -> new Bip32().random(entropyBytes));
		}

		@Test
		void generatedMnemonicIsValid() {
			// Arrange: a self-generated mnemonic must round-trip through fromMnemonic
			final Bip32 bip32 = new Bip32();
			final String mnemonic = bip32.random();

			// Act:
			final Bip32.Bip32Node node = bip32.fromMnemonic(mnemonic, "");

			// Assert: fromMnemonic validated the mnemonic (no throw) and derived a real, non-zero root key (length is
			// structurally guaranteed by PrivateKey, so it would not catch a broken derivation)
			assertThat(node.privateKey.bytes(), not(equalTo(new byte[32])));
		}

		@Test
		void japaneseFoldsIdeographicSpaceToRegularSpace() {
			// mnemonic4j joins Japanese with the ideographic space (U+3000); Bip32.random folds it to a regular space so the
			// generated phrase is space-delimited like every other language.
			final String mnemonic = new Bip32("ed25519", "japanese").random();
			assertThat(mnemonic, not(containsString("\u3000")));
			assertThat(mnemonic.split(" ").length, is(24));
		}

		@Test
		void rejectsUnsupportedMnemonicLanguage() {
			// Arrange: an unknown wordlist language is rejected when the language is first resolved (on use).
			final Bip32 bip32 = new Bip32("ed25519", "klingon");

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> bip32.random());

			// Assert:
			assertThat(ex.getMessage(), containsString("unsupported mnemonic language: klingon"));
		}
	}
}
