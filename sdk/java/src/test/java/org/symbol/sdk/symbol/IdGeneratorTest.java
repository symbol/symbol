package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.symbol.models.*;

/**
 * Tests {@link IdGenerator}; nested classes mirror the JS reference's describe blocks, plus vectors from
 * {@code tests/vectors/symbol/crypto/5.test-mosaic-id.json}.
 */
final class IdGeneratorTest {
	private static final SecureRandom RANDOM = new SecureRandom();

	// name-rejection vectors shared with the JS reference (generateMosaicAliasId / generateNamespacePath)
	private static final String[] UPPERCASE_NAMES = {
			"CAT.token", "CAT.TOKEN", "cat.TOKEN", "cAt.ToKeN", "CaT.tOkEn"
	};
	private static final String[] IMPROPER_PART_NAMES = {
			"alpha.bet@.zeta", "a!pha.beta.zeta", "alpha.beta.ze^a"
	};
	private static final String[] IMPROPER_QUALIFIED_NAMES = {
			".", "..", "...", ".a", "b.", "a..b", ".a.b", "b.a."
	};

	private record MosaicVector(long nonce, String mainnetAddress, String testnetAddress, long mainnetId, long testnetId) {
	}

	// First two entries from tests/vectors/symbol/crypto/5.test-mosaic-id.json
	// (using the public mainnet/testnet addresses + mosaic IDs).
	private static final MosaicVector[] MOSAIC_VECTORS = {
			new MosaicVector(812613930L, "NATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA34SQ33Y", "TATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA37JGO5Q",
					0x296994F01121AFC9L, 0x570FB3ED9379624CL),
			new MosaicVector(1456792364L, "NDR6EW2WBHJQDYMNGFX2UBZHMMZC5PGL2YCZOQQ", "TDR6EW2WBHJQDYMNGFX2UBZHMMZC5PGL2YBO3KA",
					0x14AA6D651D9081B4L, 0x3A334999B5C56073L),
	};

	private static Address randomAddress() {
		final byte[] bytes = new byte[Address.SIZE];
		RANDOM.nextBytes(bytes);
		return new Address(bytes);
	}

	private static void assertAllRejected(final String[] names, final Consumer<String> generate) {
		for (final String name : names) {
			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> generate.accept(name), "name: " + name);

			// Assert:
			assertThat("name: " + name, ex.getMessage(), containsString("invalid part name"));
		}
	}

	@Nested
	class GenerateMosaicId {
		@Test
		void generatesCorrectId() {
			// Act:
			final long mosaicId = IdGenerator.generateMosaicId(new Address("TATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA37JGO5Q"), 812613930L);

			// Assert:
			assertThat(mosaicId, equalTo(0x570FB3ED9379624CL));
		}

		@Test
		void matchesVectors() {
			for (final MosaicVector v : MOSAIC_VECTORS) {
				// Act:
				final long mainnet = IdGenerator.generateMosaicId(new Address(v.mainnetAddress), v.nonce);
				final long testnet = IdGenerator.generateMosaicId(new Address(v.testnetAddress), v.nonce);

				// Assert:
				assertThat(mainnet, equalTo(v.mainnetId));
				assertThat(testnet, equalTo(v.testnetId));
			}
		}

		@Test
		void producesDifferentIdsGivenDifferentAddresses() {
			// Arrange:
			final Address address1 = randomAddress();
			final Address address2 = randomAddress();

			// Act:
			final long mosaicId1 = IdGenerator.generateMosaicId(address1, 812613930L);
			final long mosaicId2 = IdGenerator.generateMosaicId(address2, 812613930L);

			// Assert:
			assertThat(mosaicId2, is(not(equalTo(mosaicId1))));
		}

		@Test
		void producesDifferentIdsGivenDifferentNonces() {
			// Arrange:
			final Address address = randomAddress();

			// Act:
			final long mosaicId1 = IdGenerator.generateMosaicId(address, 812613930L);
			final long mosaicId2 = IdGenerator.generateMosaicId(address, 812613931L);

			// Assert:
			assertThat(mosaicId2, is(not(equalTo(mosaicId1))));
		}

		@Test
		void clearsHighBit() {
			for (int i = 0; 1000 > i; ++i) {
				// Arrange:
				final Address address = randomAddress();

				// Act:
				final long mosaicId = IdGenerator.generateMosaicId(address, 812613930L);

				// Assert:
				assertThat("address: " + address, mosaicId >>> 63, equalTo(0L));
			}
		}
	}

	@Nested
	class GenerateNamespaceId {
		@Test
		void generatesCorrectId() {
			// Act:
			final long namespaceId = IdGenerator.generateNamespaceId("symbol");

			// Assert:
			assertThat(namespaceId, equalTo(0xA95F1F8A96159516L));
		}

		@Test
		void generatesCorrectChildId() {
			// Act:
			final long namespaceId = IdGenerator.generateNamespaceId("xym", 0xA95F1F8A96159516L);

			// Assert:
			assertThat(namespaceId, equalTo(0xE74B99BA41F4AFEEL));
		}

		@Test
		void producesDifferentIdsGivenDifferentNames() {
			// Act:
			final long namespaceId1 = IdGenerator.generateNamespaceId("symbol");
			final long namespaceId2 = IdGenerator.generateNamespaceId("Symbol");

			// Assert:
			assertThat(namespaceId2, is(not(equalTo(namespaceId1))));
		}

		@Test
		void producesDifferentIdsGivenDifferentParents() {
			// Act:
			final long namespaceId1 = IdGenerator.generateNamespaceId("symbol", 0xA95F1F8A96159516L);
			final long namespaceId2 = IdGenerator.generateNamespaceId("symbol", 0xA95F1F8A96159517L);

			// Assert:
			assertThat(namespaceId2, is(not(equalTo(namespaceId1))));
		}

		@Test
		void setsHighBit() {
			for (long i = 0; 1000 > i; ++i) {
				// Act:
				final long namespaceId = IdGenerator.generateNamespaceId("symbol", i);

				// Assert:
				assertThat("i: " + i, namespaceId >>> 63, equalTo(1L));
			}
		}

		@Test
		void failsIfNameContainsNamespaceSeparator() {
			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
					() -> IdGenerator.generateNamespaceId("symbol.xym"));

			// Assert:
			assertThat(ex.getMessage(),
					equalTo("'name' cannot contain '.'; if symbol.xym is a namespace path, consider using generateNamespacePath"));
		}
	}

	@Nested
	class GenerateMosaicAliasId {
		@Test
		void generatesCorrectId() {
			// Act:
			final long mosaicId = IdGenerator.generateMosaicAliasId("cat.token");

			// Assert:
			assertThat(mosaicId, equalTo(0xA029E100621B2E33L));
		}

		@Test
		void supportsMultilevelMosaics() {
			// Act:
			final long mosaicId = IdGenerator.generateMosaicAliasId("foo.bar.baz.xyz");

			// Assert:
			final long namespaceId = IdGenerator.generateNamespaceId("baz",
					IdGenerator.generateNamespaceId("bar", IdGenerator.generateNamespaceId("foo")));
			final long expectedMosaicId = IdGenerator.generateNamespaceId("xyz", namespaceId);
			assertThat(mosaicId, equalTo(expectedMosaicId));
		}

		@Test
		void returnsLastInPath() {
			// Act:
			final long alias = IdGenerator.generateMosaicAliasId("symbol.xym");
			final List<Long> path = IdGenerator.generateNamespacePath("symbol.xym");

			// Assert:
			assertThat(alias, equalTo(path.get(path.size() - 1)));
		}

		@Test
		void rejectsUppercaseCharacters() {
			assertAllRejected(UPPERCASE_NAMES, IdGenerator::generateMosaicAliasId);
		}

		@Test
		void rejectsImproperPartNames() {
			assertAllRejected(IMPROPER_PART_NAMES, IdGenerator::generateMosaicAliasId);
		}

		@Test
		void rejectsImproperQualifiedNames() {
			assertAllRejected(IMPROPER_QUALIFIED_NAMES, IdGenerator::generateMosaicAliasId);
		}

		@Test
		void rejectsEmptyString() {
			assertAllRejected(new String[]{
					""
			}, IdGenerator::generateMosaicAliasId);
		}
	}

	@Nested
	class IsMosaicAlias {
		@Test
		void onlyReturnsTrueWhenMosaicIdIsAlias() {
			// Act:
			final boolean highBitUnsetIsAlias = IdGenerator.isMosaicAlias(0x7FFFFFFFFFFFFFFFL);
			final boolean midValueIsAlias = IdGenerator.isMosaicAlias(0x0FFFFFFFFFFFFFFFL);
			final boolean highBitSetIsAlias = IdGenerator.isMosaicAlias(0x8FFFFFFFFFFFFFFFL);
			final boolean maxValueIsAlias = IdGenerator.isMosaicAlias(0xFFFFFFFFFFFFFFFFL);
			final boolean generatedAliasIsAlias = IdGenerator.isMosaicAlias(IdGenerator.generateMosaicAliasId("cat.token"));

			// Assert: high-bit unset => false, high-bit set => true
			assertThat(highBitUnsetIsAlias, is(false));
			assertThat(midValueIsAlias, is(false));
			assertThat(highBitSetIsAlias, is(true));
			assertThat(maxValueIsAlias, is(true));
			assertThat(generatedAliasIsAlias, is(true));
		}
	}

	@Nested
	class IsValidNamespaceName {
		private void assertAllNamesValid(final String[] names, final boolean expected) {
			for (final String name : names) {
				// Act:
				final boolean isValid = IdGenerator.isValidNamespaceName(name);

				// Assert:
				assertThat("name: " + name, isValid, is(expected));
			}
		}

		@Test
		void returnsTrueWhenAllCharactersAreAlphanumeric() {
			assertAllNamesValid(new String[]{
					"a", "be", "cat", "doom", "09az09", "az09az"
			}, true);
		}

		@Test
		void returnsTrueWhenNameContainsSeparator() {
			assertAllNamesValid(new String[]{
					"al-ce", "al_ce", "alice-", "alice_"
			}, true);
		}

		@Test
		void returnsFalseWhenNameStartsWithSeparator() {
			assertAllNamesValid(new String[]{
					"-alice", "_alice"
			}, false);
		}

		@Test
		void returnsFalseWhenAnyCharacterIsInvalid() {
			assertAllNamesValid(new String[]{
					"al.ce", "alIce", "al ce", "al@ce", "al#ce"
			}, false);
		}

		@Test
		void returnsFalseForNullAndEmpty() {
			// Act:
			final boolean nullIsValid = IdGenerator.isValidNamespaceName(null);
			final boolean emptyIsValid = IdGenerator.isValidNamespaceName("");

			// Assert: Java-only null-safety (the JS reference cannot pass null)
			assertThat(nullIsValid, is(false));
			assertThat(emptyIsValid, is(false));
		}
	}

	@Nested
	class GenerateNamespacePath {
		@Test
		void generatesCorrectRootId() {
			// Act:
			final List<Long> path = IdGenerator.generateNamespacePath("cat");

			// Assert:
			assertThat(path, equalTo(List.of(0xB1497F5FBA651B4FL)));
		}

		@Test
		void generatesCorrectChildId() {
			// Act:
			final List<Long> path = IdGenerator.generateNamespacePath("cat.token");

			// Assert:
			assertThat(path, equalTo(List.of(0xB1497F5FBA651B4FL, 0xA029E100621B2E33L)));
		}

		@Test
		void supportsMultilevelNamespaces() {
			// Act:
			final List<Long> path = IdGenerator.generateNamespacePath("foo.bar.baz.xyz");

			// Assert:
			final List<Long> expectedPath = new ArrayList<>();
			expectedPath.add(IdGenerator.generateNamespaceId("foo"));
			for (final String name : new String[]{
					"bar", "baz", "xyz"
			})
				expectedPath.add(IdGenerator.generateNamespaceId(name, expectedPath.get(expectedPath.size() - 1)));

			assertThat(path, equalTo(expectedPath));
		}

		@Test
		void rejectsUppercaseCharacters() {
			assertAllRejected(UPPERCASE_NAMES, IdGenerator::generateNamespacePath);
		}

		@Test
		void rejectsImproperPartNames() {
			assertAllRejected(IMPROPER_PART_NAMES, IdGenerator::generateNamespacePath);
		}

		@Test
		void rejectsImproperQualifiedNames() {
			assertAllRejected(IMPROPER_QUALIFIED_NAMES, IdGenerator::generateNamespacePath);
		}

		@Test
		void rejectsEmptyString() {
			assertAllRejected(new String[]{
					""
			}, IdGenerator::generateNamespacePath);
		}
	}
}
