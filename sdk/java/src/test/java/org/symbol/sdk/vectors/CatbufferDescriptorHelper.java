package org.symbol.sdk.vectors;

import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.symbol.sdk.utils.Converter;

/**
 * Shared helpers for descriptor-driven catbuffer vector tests. Operates on the parsed JSON tree and returns the same shape — never the SDK
 * model types — so callers can keep walking nested descriptors.
 */
final class CatbufferDescriptorHelper {

	private CatbufferDescriptorHelper() {
	}

	/**
	 * Normalizes a raw vector descriptor into the input shape the descriptor pipeline expects: keys camel-cased to match {@code setField}
	 * names, and integer values narrowed to {@link Long} (every model integer is {@code long}-backed).
	 *
	 * @param descriptor Source descriptor.
	 * @return Normalized deep copy.
	 */
	static Map<String, Object> normalizeInput(final Map<?, ?> descriptor) {
		final Map<String, Object> out = new LinkedHashMap<>();
		for (Map.Entry<?, ?> entry : descriptor.entrySet())
			out.put(makeCamelCase((String) entry.getKey()), normalizeValue(entry.getValue()));

		return out;
	}

	private static Object normalizeValue(final Object value) {
		if (value instanceof Number number)
			return Converter.toLong(number);

		if (value instanceof Map<?, ?> map)
			return normalizeInput(map);

		if (value instanceof List<?> list)
			return list.stream().map(CatbufferDescriptorHelper::normalizeValue).toList();

		return value;
	}

	/**
	 * Converts a {@code snake_case} string into {@code camelCase}.
	 *
	 * @param name Snake-case string.
	 * @return Camel-cased string.
	 */
	static String makeCamelCase(final String name) {
		final StringBuilder out = new StringBuilder(name.length());
		boolean upperNext = false;
		for (int i = 0; i < name.length(); ++i) {
			final char ch = name.charAt(i);
			if ('_' == ch) {
				upperNext = true;
				continue;
			}

			out.append(upperNext ? Character.toUpperCase(ch) : ch);
			upperNext = false;
		}
		return out.toString();
	}

	/**
	 * Walks {@code descriptor} (including nested maps/lists) in place, replacing every hex {@link String} value with the corresponding
	 * {@code byte[]}; excludes the false-positive {@code value} field of {@code namespace_metadata_transaction_v1}.
	 *
	 * @param descriptor Descriptor to fix up.
	 */
	static void fixupDescriptorCommon(final Map<String, Object> descriptor) {
		final Object type = descriptor.get("type");
		descriptor.replaceAll((key, value) -> isPlainTextValue(type, key) ? value : fixupValue(value));
	}

	private static Object fixupValue(final Object value) {
		if (value instanceof String string && Converter.isHexString(string))
			return Converter.hexToUint8(string);

		if (value instanceof Map<?, ?> map) {
			final Object type = map.get("type");
			final Map<String, Object> fixed = new LinkedHashMap<>();
			map.forEach((key, nested) -> fixed.put((String) key, isPlainTextValue(type, (String) key) ? nested : fixupValue(nested)));
			return fixed;
		}

		if (value instanceof List<?> list)
			return list.stream().map(CatbufferDescriptorHelper::fixupValue).toList();

		return value;
	}

	/**
	 * True when a hex-looking string value is deliberately plain text in the vectors (currently only the namespace metadata {@code value}
	 * field, e.g. "ABC123"); the single home for this exception, shared by the tree fixup and the typed-descriptor mappers.
	 *
	 * @param type Descriptor type of the enclosing object.
	 * @param key Field name.
	 * @return Whether the value must stay text.
	 */
	static boolean isPlainTextValue(final Object type, final String key) {
		return "value".equals(key) && "namespace_metadata_transaction_v1".equals(type);
	}

	/**
	 * Raw-byte view of a byte-carrying vector value: hex strings decode to their bytes, anything else is UTF-8 text (the same rule
	 * {@link #fixupDescriptorCommon} applies tree-wide, exposed for the typed-descriptor mappers).
	 *
	 * @param value Vector value.
	 * @return Byte view of the value.
	 */
	static byte[] rawBytes(final Object value) {
		final String string = (String) value;
		return Converter.isHexString(string) ? Converter.hexToUint8(string) : string.getBytes(StandardCharsets.UTF_8);
	}
}
