package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.aMapWithSize;
import static org.hamcrest.Matchers.equalTo;

import java.util.Map;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class CharMappingTest {
	@Nested
	final class Builder {
		@Test
		void initiallyHasEmptyMap() {
			// Arrange:
			final CharMapping.CharacterMapBuilder builder = CharMapping.createBuilder();

			// Act:
			final Map<Character, Byte> map = builder.map();

			// Assert:
			assertThat(map, aMapWithSize(0));
		}

		@Test
		void canAddSingleArbitraryRangeWithZeroBase() {
			// Arrange:
			final CharMapping.CharacterMapBuilder builder = CharMapping.createBuilder();

			// Act:
			builder.addRange('d', 'f', 0);
			final Map<Character, Byte> map = builder.map();

			// Assert:
			assertThat(map, equalTo(Map.of('d', (byte) 0, 'e', (byte) 1, 'f', (byte) 2)));
		}

		@Test
		void canAddSingleArbitraryRangeWithNonzeroBase() {
			// Arrange:
			final CharMapping.CharacterMapBuilder builder = CharMapping.createBuilder();

			// Act:
			builder.addRange('d', 'f', 17);
			final Map<Character, Byte> map = builder.map();

			// Assert:
			assertThat(map, equalTo(Map.of('d', (byte) 17, 'e', (byte) 18, 'f', (byte) 19)));
		}

		@Test
		void canAddMultipleArbitraryRanges() {
			// Arrange:
			final CharMapping.CharacterMapBuilder builder = CharMapping.createBuilder();

			// Act:
			builder.addRange('b', 'b', 8);
			builder.addRange('d', 'f', 17);
			builder.addRange('y', 'z', 0);
			final Map<Character, Byte> map = builder.map();

			// Assert:
			assertThat(map, equalTo(Map.of('b', (byte) 8, 'd', (byte) 17, 'e', (byte) 18, 'f', (byte) 19, 'y', (byte) 0, 'z', (byte) 1)));
		}

		@Test
		void canAddMultipleArbitraryOverlappingRanges() {
			// Arrange:
			final CharMapping.CharacterMapBuilder builder = CharMapping.createBuilder();

			// Act:
			builder.addRange('b', 'b', 18);
			builder.addRange('d', 'f', 17);
			builder.addRange('y', 'z', 19);
			final Map<Character, Byte> map = builder.map();

			// Assert:
			assertThat(map,
					equalTo(Map.of('b', (byte) 18, 'd', (byte) 17, 'e', (byte) 18, 'f', (byte) 19, 'y', (byte) 19, 'z', (byte) 20)));
		}
	}
}
