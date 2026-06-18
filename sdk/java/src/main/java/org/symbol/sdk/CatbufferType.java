package org.symbol.sdk;

import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.function.Function;

import org.symbol.sdk.utils.Writer;

/**
 * Shared base for every generated catbuffer struct (PODs and enums do not extend it): {@link #serialize()} allocates a {@link Writer} sized
 * by {@link #size()} and threads it through {@link #serializeInto(Writer)}.
 */
public abstract class CatbufferType implements Serializer {
	/**
	 * * Calculates the size of the object when serialized. Every generated struct overrides.
	 *
	 * @return bytes.
	 */
	@Override
	public final byte[] serialize() {
		final Writer buffer = new Writer(this.size());
		serializeInto(buffer);
		return buffer.storage;
	}

	/**
	 * Encodes this object's fields into the supplied writer. Subclasses extending an abstract struct must call
	 * {@code super.serializeInto(buffer)} before writing their own fields so parent-defined headers land in canonical order.
	 *
	 * @param buffer Output writer pre-sized to {@link #size()}.
	 */
	protected abstract void serializeInto(Writer buffer);

	/**
	 * Returns the JSON-serializable projection of this object (nested {@code Map}/{@code List} tree with string-encoded u64 values). Every
	 * generated struct overrides; the projection is a wire snapshot, not a descriptor — byte fields render as hex while descriptor input
	 * reads strings as UTF-8 text.
	 *
	 * @return JSON-serializable representation of this object.
	 */
	public Object toJson() {
		throw new UnsupportedOperationException(getClass().getName() + " does not implement toJson");
	}

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
	protected static <T> java.util.List<T> asList(final Object value, final Class<T> elementType) {
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
	protected static byte[] asBytes(final Object value) {
		if (null == value)
			return null;

		if (value instanceof byte[] bytes)
			return bytes;

		if (value instanceof String string)
			return string.getBytes(StandardCharsets.UTF_8);

		if (value instanceof ByteArray byteArray)
			return byteArray.bytes();

		throw new InvalidDescriptorException("cannot adapt " + value.getClass().getName() + " to byte[]");
	}

	/**
	 * Convert a descriptor value into a {@link ByteArray} subtype: short-circuits when the value already matches the target type, otherwise
	 * rewraps the underlying bytes via the supplied constructor reference.
	 *
	 * @param value Source value.
	 * @param target Target class (also used for {@link Class#isInstance} short-circuit).
	 * @param ctor Byte-array constructor reference for the target type.
	 * @param <T> Target type.
	 * @return Adapted value (or {@code null} when {@code value} is {@code null}).
	 */
	protected static <T extends ByteArray> T asByteArray(final Object value, final Class<T> target, final Function<byte[], T> ctor) {
		if (null == value)
			return null;

		if (target.isInstance(value))
			return target.cast(value);

		if (value instanceof ByteArray byteArray)
			return ctor.apply(byteArray.bytes());

		if (value instanceof byte[] bytes)
			return ctor.apply(bytes);

		throw new InvalidDescriptorException(String.format("cannot adapt %s to %s", value.getClass().getName(), target.getSimpleName()));
	}

	/**
	 *
	 * Convert a descriptor value to {@code int}. Accepts any {@link Number}.
	 *
	 * @param value Source value.
	 * @return Integer value.
	 */
	protected static int asInt(final Object value) {
		if (value instanceof Number number)
			return number.intValue();

		throw new InvalidDescriptorException("cannot adapt " + value.getClass().getName() + " to int");
	}

	/**
	 * Convert a descriptor value to {@code long}. Accepts any {@link Number}.
	 *
	 * @param value Source value.
	 * @return Long value.
	 */
	protected static long asLong(final Object value) {
		if (value instanceof Number number)
			return number.longValue();

		throw new InvalidDescriptorException("cannot adapt " + value.getClass().getName() + " to long");
	}

	/**
	 * Convert a descriptor value to {@code short}. Accepts any {@link Number}.
	 *
	 * @param value Source value.
	 * @return Short value.
	 */
	protected static short asShort(final Object value) {
		if (value instanceof Number number)
			return number.shortValue();

		throw new InvalidDescriptorException("cannot adapt " + value.getClass().getName() + " to short");
	}

	/**
	 * Convert a descriptor value to {@code byte}. Accepts any {@link Number}.
	 *
	 * @param value Source value.
	 * @return Byte value.
	 */
	protected static byte asByte(final Object value) {
		if (value instanceof Number number)
			return number.byteValue();

		throw new InvalidDescriptorException("cannot adapt " + value.getClass().getName() + " to byte");
	}

	// endregion
}
