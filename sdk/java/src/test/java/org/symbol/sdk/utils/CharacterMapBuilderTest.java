package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;

import java.util.HashMap;
import java.util.Map;
import org.junit.Test;

public class CharacterMapBuilderTest {
	// region builder

	@Test
	public void canCreateEmptyMap() {
		// Arrange:
		final CharacterMapBuilder builder = CharacterMapBuilder.create();

		// Act:
		final Map<Character, Byte> charMap = builder.build();

		// Assert:
		assertThat("Character map should be empty", charMap.isEmpty());
	}

	@Test
	public void canCreateMapWithSingleArbitraryRangeZeroBase() {
		// Arrange:
		final CharacterMapBuilder builder = CharacterMapBuilder.create(b -> b.addRange('d', 'f', (byte) 0));
		final Map<Character, Byte> expectedMap = new HashMap<Character, Byte>() {
			{
				put('d', (byte) 0);
				put('e', (byte) 1);
				put('f', (byte) 2);
			}
		};

		// Act:
		final Map<Character, Byte> charMap = builder.build();

		// Assert:
		assertThat(charMap, equalTo(expectedMap));
	}

	@Test
	public void canCreateMapWithSingleArbitraryRangeNonZeroBase() {
		// Arrange:
		final CharacterMapBuilder builder = CharacterMapBuilder.create(b -> b.addRange('a', 'c', (byte) 17));
		final Map<Character, Byte> expectedMap = new HashMap<Character, Byte>() {
			{
				put('a', (byte) 17);
				put('b', (byte) 18);
				put('c', (byte) 19);
			}
		};

		// Act:
		final Map<Character, Byte> charMap = builder.build();

		// Assert:
		assertThat(charMap, equalTo(expectedMap));
	}

	@Test
	public void canCreateMapWithMultipleArbitrary() {
		// Arrange:
		final CharacterMapBuilder builder = CharacterMapBuilder.create(b -> {
			b.addRange('b', 'b', (byte) 8);
			b.addRange('d', 'f', (byte) 17);
			b.addRange('y', 'z', (byte) 0);
		});
		final Map<Character, Byte> expectedMap = new HashMap<Character, Byte>() {
			{
				put('b', (byte) 8);
				put('d', (byte) 17);
				put('e', (byte) 18);
				put('f', (byte) 19);
				put('y', (byte) 0);
				put('z', (byte) 1);
			}
		};

		// Act:
		final Map<Character, Byte> charMap = builder.build();

		// Assert:
		assertThat(charMap, equalTo(expectedMap));
	}

	@Test
	public void canCreateMapWithMultipleArbitraryOverlappingRange() {
		// Arrange:
		final CharacterMapBuilder builder = CharacterMapBuilder.create(b -> {
			b.addRange('b', 'b', (byte) 18);
			b.addRange('d', 'f', (byte) 17);
			b.addRange('y', 'z', (byte) 19);
		});
		final Map<Character, Byte> expectedMap = new HashMap<Character, Byte>() {
			{
				put('b', (byte) 18);
				put('d', (byte) 17);
				put('e', (byte) 18);
				put('f', (byte) 19);
				put('y', (byte) 19);
				put('z', (byte) 20);
			}
		};

		// Act:
		final Map<Character, Byte> charMap = builder.build();

		// Assert:
		assertThat(charMap, equalTo(expectedMap));
	}

	// endregion
}
