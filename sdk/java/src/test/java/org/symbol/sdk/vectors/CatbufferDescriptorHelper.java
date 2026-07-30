package org.symbol.sdk.vectors;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
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
			// narrow every JSON integer to the model's long backing; Converter.toLong keeps the full u64 bit
			// pattern (values >= 2^63 become the negative long) and rejects out-of-range / non-integer numbers
			return Converter.toLong(number);

		if (value instanceof Map<?, ?> map)
			return normalizeInput(map);

		if (value instanceof List<?> list) {
			final List<Object> out = new ArrayList<>(list.size());
			// recurse through every element so scalar Number members (e.g. an array of mosaic ids) narrow too
			for (Object element : list)
				out.add(normalizeValue(element));

			return out;
		}

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
		for (Map.Entry<String, Object> entry : descriptor.entrySet()) {
			if (isPlainTextValue(type, entry.getKey()))
				continue;

			entry.setValue(fixupValue(entry.getValue()));
		}
	}

	private static Object fixupValue(final Object value) {
		if (value instanceof String string && Converter.isHexString(string))
			return Converter.hexToUint8(string);

		if (value instanceof Map<?, ?> map) {
			// rebuild rather than mutate so the walk stays type-checked; the tree is the fresh
			// copy produced by normalizeInput, so identity is not load-bearing.
			final Map<String, Object> fixed = new LinkedHashMap<>();
			final Object type = map.get("type");
			map.forEach((key, nested) -> {
				final String fieldName = (String) key;
				fixed.put(fieldName, isPlainTextValue(type, fieldName) ? nested : fixupValue(nested));
			});
			return fixed;
		}

		if (value instanceof List<?> list) {
			final List<Object> fixed = new ArrayList<>(list.size());
			for (Object element : list)
				fixed.add(fixupValue(element));
			return fixed;
		}

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
