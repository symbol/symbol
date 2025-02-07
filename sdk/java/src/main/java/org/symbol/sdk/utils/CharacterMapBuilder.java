package org.symbol.sdk.utils;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;

/**
 * Builder for building a character map.
 */
public class CharacterMapBuilder {
	private final Map<Character, Byte> charMap = new HashMap<>();

	// prevent external instantiation
	private CharacterMapBuilder() {
	}

	private Map<Character, Byte> getMap() {
		return charMap;
	}

	/**
	 * Adds a range mapping to the map.
	 *
	 * @param start Start character.
	 * @param end End character.
	 * @param base Value corresponding to the start character.
	 */
	public void addRange(final char start, final char end, final byte base) {
		final byte startCode = (byte) start;
		final byte endCode = (byte) end;

		for (byte code = startCode; code <= endCode; ++code)
			this.charMap.put((char) code, (byte) (code - startCode + base));
	}

	/**
	 * Creates an instance of {@link CharacterMapBuilder}
	 *
	 * @return an instance of {@link CharacterMapBuilder}
	 */
	public static CharacterMapBuilder create() {
		return new CharacterMapBuilder();
	}

	/**
	 * Creates an instance of {@link CharacterMapBuilder}
	 *
	 * @param characterMapBuilderConsumer Consumer to initialize the map.
	 * @return an instance of {@link CharacterMapBuilder}
	 */
	public static CharacterMapBuilder create(final Consumer<CharacterMapBuilder> characterMapBuilderConsumer) {
		final CharacterMapBuilder builder = create();
		characterMapBuilderConsumer.accept(builder);
		return builder;
	}

	/**
	 * Builds character map.
	 *
	 * @return the character {@link Map} of {@link Character} to {@link Byte}
	 */
	public Map<Character, Byte> build() {
		return getMap();
	}
}
