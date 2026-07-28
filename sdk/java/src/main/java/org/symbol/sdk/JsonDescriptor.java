package org.symbol.sdk;

import java.util.LinkedHashMap;
import java.util.Map;

import com.fasterxml.jackson.core.JacksonException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.json.JsonMapper;

/**
 * Parses a JSON document into the descriptor {@code Map} the facade {@code createTransactionFromDescriptor} methods consume; integral
 * numbers deserialize as {@link Integer}/{@link Long}/{@link java.math.BigInteger} by magnitude — the descriptor coercions accept all
 * three. Note that descriptor byte fields read strings as UTF-8, not the hex of the model {@code toJson()} projection.
 */
public final class JsonDescriptor {
	// NOT using USE_LONG_FOR_INTS: it hard-fails on integers above Long.MAX instead of promoting them to BigInteger,
	// which would reject any bare-number u64 with its high bit set (e.g. every metadata scopedMetadataKey). Small
	// integers deserialize as Integer and are normalized to Long below.
	private static final ObjectMapper MAPPER = JsonMapper.builder()
			// reject content after the top-level value ('{...} garbage' silently keeps only the object otherwise), matching
			// JavaScript's JSON.parse and Python's json.loads
			.enable(DeserializationFeature.FAIL_ON_TRAILING_TOKENS).build();

	private JsonDescriptor() {
	}

	/**
	 * Parses a JSON object into a descriptor map.
	 *
	 * @param json JSON document whose top level is an object.
	 * @return Descriptor map with {@code String} keys and {@code String}/{@code Number}/{@code List}/{@code Map} values.
	 * @throws InvalidDescriptorException If the document is malformed or not a JSON object.
	 */
	public static Map<String, Object> parse(final String json) {
		try {
			final Map<String, Object> descriptor = MAPPER.readValue(json, new TypeReference<LinkedHashMap<String, Object>>() {
			});
			// the JSON literal 'null' deserializes to a null map; reject it so callers get the documented exception
			if (null == descriptor)
				throw new InvalidDescriptorException("descriptor JSON is malformed: expected a JSON object, got null");

			return descriptor;
		} catch (final JacksonException ex) {
			throw new InvalidDescriptorException("descriptor JSON is malformed: " + ex.getOriginalMessage(), ex);
		}
	}
}
