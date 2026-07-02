package org.symbol.sdk;

import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.function.Function;

import org.symbol.sdk.utils.Converter;
import org.symbol.sdk.utils.Writer;

/**
 * Shared base for every generated catbuffer struct (PODs and enums do not extend it): {@link #serialize()} allocates a {@link Writer} sized
 * by {@link #size()} and threads it through {@link #serializeInto(Writer)}.
 */
public abstract class CatbufferType implements Serializer {
	/**
	 * Serializes this object to bytes: allocates a {@link Writer} sized by {@link #size()} and threads it through
	 * {@link #serializeInto(Writer)}.
	 *
	 * @return Serialized bytes.
	 */
	@Override
	public final byte[] serialize() {
		final Writer buffer = new Writer(this.size());
		serializeInto(buffer);
		return buffer.storage();
	}

	/**
	 * Encodes this object's fields into the supplied writer. Subclasses extending an abstract struct must call
	 * {@code super.serializeInto(buffer)} before writing their own fields so parent-defined headers land in canonical order.
	 *
	 * @param buffer Output writer pre-sized to {@link #size()}.
	 */
	protected abstract void serializeInto(Writer buffer);

	/**
	 * Returns the JSON-serializable projection of this object (nested {@code Map}/{@code List} tree with string-encoded u64 values); the
	 * projection is a wire snapshot, not a descriptor — byte fields render as hex while descriptor input reads strings as UTF-8 text.
	 *
	 * @return JSON-serializable representation of this object.
	 */
	public abstract Object toJson();

	/**
	 * Serializes {@link #toJson()} to a JSON document.
	 *
	 * @return JSON document string.
	 */
	public final String toJsonString() {
		try {
			return JSON_MAPPER.writeValueAsString(toJson());
		} catch (final com.fasterxml.jackson.core.JsonProcessingException ex) {
			throw new IllegalStateException("toJson() of " + getClass().getName() + " is not JSON-serializable", ex);
		}
	}

	private static final com.fasterxml.jackson.databind.ObjectMapper JSON_MAPPER = new com.fasterxml.jackson.databind.ObjectMapper();

	// region descriptor surface

	/**
	 * Sorts the collections of this object so that it can be serialized canonically. Default is a no-op; structs with sort-key typed-array
	 * fields override.
	 */
	public void sort() {
	}

	/**
	 * Returns the type-hint map used by the descriptor pipeline to pick a rule per key; default is an empty map.
	 *
	 * @return Field-name → rule-key map.
	 */
	public Map<String, String> typeHints() {
		return Map.of();
	}

	/**
	 * Sets a named field. Generated subclasses override with a {@code switch} whose {@code default} arm chains to {@code super.setField}
	 * for inherited fields; the base implementation throws.
	 *
	 * @param name Field name (descriptor key).
	 * @param value Value to assign.
	 */
	public void setField(final String name, final Object value) {
		throw new InvalidDescriptorException(String.format("no field \"%s\" on %s", name, getClass().getSimpleName()));
	}

	/**
	 * Reads a named field. Generated subclasses override with a {@code switch} chaining to {@code super.getField(name)}.
	 *
	 * @param name Field name.
	 * @return Field value (callers cast to the appropriate type).
	 */
	public Object getField(final String name) {
		throw new InvalidDescriptorException(String.format("no field \"%s\" on %s", name, getClass().getSimpleName()));
	}

	// endregion

	// region convert helpers used by generated setField switches

	/**
	 * Convert a descriptor value to a mutable typed list, validating every element — generated {@code setField} arms use this instead of an
	 * unchecked {@code (List<T>)} cast so a wrongly-typed element fails here instead of during serialization.
	 *
	 * @param <T> Element type.
	 * @param value Raw descriptor value.
	 * @param elementType Expected element class.
	 * @return Mutable list of checked elements, or {@code null} when {@code value} is null.
	 */
	protected static final <T> java.util.List<T> asList(final Object value, final Class<T> elementType) {
		if (null == value)
			return null;

		final java.util.List<?> list = (java.util.List<?>) value;
		final java.util.List<T> result = new java.util.ArrayList<>(list.size());
		for (final Object element : list)
			result.add(elementType.cast(element));

		return result;
	}

	/**
	 * Convert a descriptor value to {@code byte[]}: accepts {@code byte[]} as-is, UTF-8-encodes {@link String}s, unwraps any
	 * {@link ByteArray}, and passes {@code null} through.
	 *
	 * @param value Source value.
	 * @return Byte array view of the value.
	 */
	protected static final byte[] asBytes(final Object value) {
		if (null == value)
			return null;

		if (value instanceof byte[] bytes)
			return bytes;

		if (value instanceof String string)
			return string.getBytes(StandardCharsets.UTF_8);

		if (value instanceof ByteArray byteArray)
			return byteArray.bytes();

		throw new IllegalArgumentException("cannot convert " + value.getClass().getName() + " to byte[]");
	}

	/**
	 * Convert a descriptor value into a {@link ByteArray} subtype: short-circuits when the value already matches the target type, otherwise
	 * rewraps the underlying bytes of a same-named ByteArray twin (across packages) or a raw {@code byte[]} via the supplied constructor
	 * reference; an unrelated ByteArray type is rejected.
	 *
	 * @param value Source value.
	 * @param target Target class (also used for {@link Class#isInstance} short-circuit).
	 * @param ctor Byte-array constructor reference for the target type.
	 * @param <T> Target type.
	 * @return Adapted value (or {@code null} when {@code value} is {@code null}).
	 */
	protected static final <T extends ByteArray> T asByteArray(final Object value, final Class<T> target, final Function<byte[], T> ctor) {
		if (null == value)
			return null;

		if (target.isInstance(value))
			return target.cast(value);

		// bridge a same-named ByteArray across packages: hand-written CryptoTypes values adapt to their generated model twin
		// (e.g. CryptoTypes.PublicKey -> models.PublicKey).
		if (value instanceof ByteArray byteArray && value.getClass().getSimpleName().equals(target.getSimpleName()))
			return ctor.apply(byteArray.bytes());

		if (value instanceof byte[] bytes)
			return ctor.apply(bytes);

		throw new IllegalArgumentException(String.format("cannot convert %s to %s", value.getClass().getName(), target.getSimpleName()));
	}

	/**
	 * Converts a descriptor value to an exact {@code int} via {@link Converter#toInt(Number)} / {@link Converter#toInt(String)}: a
	 * fixed-width integral wrapper or a decimal/{@code 0x}-hex string, rejecting out-of-range values rather than truncating.
	 *
	 * @param value Source value.
	 * @return Integer value.
	 */
	protected static final int asInt(final Object value) {
		if (value instanceof String string)
			return Converter.toInt(string);

		if (value instanceof Number number)
			return Converter.toInt(number);

		throw new IllegalArgumentException("cannot convert " + (null == value ? "null" : value.getClass().getName()) + " to int");
	}

	/**
	 * Converts a descriptor value to a {@code long} via {@link Converter#toLong(Number)} / {@link Converter#toLong(String)}: a fixed-width
	 * integral wrapper or a decimal/{@code 0x}-hex string ({@link java.math.BigInteger} and non-integral {@link Float}/{@link Double} are
	 * rejected — pass a u64 {@code >= 2^63} as a string).
	 *
	 * @param value Source value.
	 * @return Long value.
	 */
	protected static final long asLong(final Object value) {
		if (value instanceof String string)
			return Converter.toLong(string);

		if (value instanceof Number number)
			return Converter.toLong(number);

		throw new IllegalArgumentException("cannot convert " + (null == value ? "null" : value.getClass().getName()) + " to long");
	}

	// endregion
}
