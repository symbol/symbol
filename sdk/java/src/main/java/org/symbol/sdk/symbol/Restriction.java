package org.symbol.sdk.symbol;

import java.nio.charset.StandardCharsets;

import org.symbol.sdk.utils.Converter;
import org.symbol.sdk.utils.Transforms;

/**
 * Symbol mosaic restriction key generation.
 */
public final class Restriction {
	private Restriction() {
	}

	/**
	 * Generates a mosaic restriction key from a string seed: the first 8 bytes of {@code SHA3-256(seed)} read as a little-endian unsigned
	 * 64-bit integer.
	 *
	 * @param seed Mosaic restriction key seed.
	 * @return Mosaic restriction key (64-bit two's-complement bit pattern).
	 */
	public static long mosaicRestrictionGenerateKey(final String seed) {
		final byte[] digest = Transforms.sha3_256(seed.getBytes(StandardCharsets.UTF_8));

		return Converter.bytesToInt(digest, 8, false);
	}
}
