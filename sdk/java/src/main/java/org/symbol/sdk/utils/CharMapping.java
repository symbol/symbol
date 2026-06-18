package org.symbol.sdk.utils;

import java.util.HashMap;
import java.util.Map;

/**
 * Builder for building a character map.
 */
public final class CharMapping {
	private CharMapping() {
	}

	/**
	 * Creates a builder for building a character map.
	 *
	 * @return Character map builder.
	 */
	public static CharacterMapBuilder createBuilder() {
		return new CharacterMapBuilder();
	}

	/**
	 * Builder used to assemble a character-to-value map by ranges.
	 */
	public static final class CharacterMapBuilder {
		/**
		 * Mapping of characters to character codes.
		 */
		private final Map<Character, Byte> map = new HashMap<>();

		/**
		 * Creates a new character map builder.
		 */
		public CharacterMapBuilder() {
		}

		/**
		 * Adds a range mapping to the map.
		 *
		 * @param start Start character.
		 * @param end End character.
		 * @param base Value corresponding to the start character.
		 */
		public void addRange(final char start, final char end, final int base) {
			for (int code = start; code <= end; ++code)
				map.put((char) code, (byte) (code - start + base));
		}

		/**
		 * Returns the assembled character map.
		 *
		 * @return Character map.
		 */
		public Map<Character, Byte> map() {
			return map;
		}
	}
}
