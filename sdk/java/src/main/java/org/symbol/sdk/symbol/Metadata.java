package org.symbol.sdk.symbol;

import java.nio.charset.StandardCharsets;

import org.symbol.sdk.utils.Converter;
import org.symbol.sdk.utils.Transforms;

/**
 * Symbol metadata helpers.
 */
public final class Metadata {
	private Metadata() {
	}

	/**
	 * Generates a metadata key from a string seed: the first 8 bytes of {@code SHA3-256(seed)} read as a unsigned 64-bit integer with the
	 * top bit forced to one (matching the SDK V2 implementation).
	 *
	 * @param seed Metadata key seed.
	 * @return Metadata key (64-bit two's-complement bit pattern).
	 */
	public static long generateKey(final String seed) {
		final byte[] digest = Transforms.sha3_256(seed.getBytes(StandardCharsets.UTF_8));

		return Converter.bytesToInt(digest, 8, false) | (1L << 63);
	}

	/**
	 * Creates a metadata payload that updates {@code oldValue} to {@code newValue}: the XOR of the two values up to the shorter length,
	 * followed by the remaining bytes of the longer value.
	 *
	 * @param oldValue Old metadata value, or {@code null} when none exists.
	 * @param newValue New metadata value.
	 * @return Metadata payload.
	 */
	public static byte[] updateValue(final byte[] oldValue, final byte[] newValue) {
		if (null == oldValue)
			return newValue.clone();

		// the result is the longer input with its leading bytes XORed against the shorter input (the tail past the shorter
		// length is left as-is); XOR is commutative, so which input is "longer" only decides the untouched tail
		final byte[] longer = oldValue.length >= newValue.length ? oldValue : newValue;
		final byte[] shorter = oldValue.length >= newValue.length ? newValue : oldValue;

		final byte[] result = longer.clone();
		for (int i = 0; i < shorter.length; ++i)
			result[i] ^= shorter[i];

		return result;
	}
}
