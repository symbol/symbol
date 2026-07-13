package org.symbol.sdk.symbol;

import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

import org.symbol.sdk.utils.Converter;
import org.symbol.sdk.utils.Transforms;

/**
 * Mosaic and namespace identifier generation. All identifiers are 64-bit unsigned values, represented as the raw {@code long} bit pattern
 * (so ids {@code >= 2^63} are negative; interpret via {@link Long#toUnsignedString(long)}).
 */
public final class IdGenerator {
	private static final long NAMESPACE_FLAG = 1L << 63;
	private static final Pattern VALID_NAMESPACE_PATTERN = Pattern.compile("^[a-z0-9][a-z0-9_-]*$");

	private IdGenerator() {
	}

	private static byte[] uint32ToBytes(final long value) {
		return Converter.intToBytes(value, 4);
	}

	private static long digestToUint64(final byte[] digest) {
		// the first 8 bytes of the digest, are the 64-bit id bit pattern
		return Converter.bytesToInt(digest, 8, false);
	}

	/**
	 * Generates a mosaic id from an owner address and a nonce.
	 *
	 * @param ownerAddress Owner address.
	 * @param nonce Nonce.
	 * @return Computed mosaic id (high bit cleared).
	 */
	public static long generateMosaicId(final Address ownerAddress, final long nonce) {
		final byte[] digest = Transforms.sha3_256(uint32ToBytes(nonce), ownerAddress.bytes());

		// clear the namespace flag (high bit) so a mosaic id is never mistaken for an alias
		return digestToUint64(digest) & ~NAMESPACE_FLAG;
	}

	/**
	 * Generates a namespace id from a name and an optional parent namespace id.
	 *
	 * @param name Namespace name.
	 * @param parentNamespaceId Parent namespace id (use {@code 0} for root).
	 * @return Computed namespace id (high bit set).
	 */
	public static long generateNamespaceId(final String name, final long parentNamespaceId) {
		if (name.contains("."))
			throw new IllegalArgumentException(
					String.format("'name' cannot contain '.'; if %s is a namespace path, consider using generateNamespacePath", name));

		final long low = parentNamespaceId & 0xFFFFFFFFL;
		final long high = (parentNamespaceId >>> 32) & 0xFFFFFFFFL;

		final byte[] digest = Transforms.sha3_256(uint32ToBytes(low), uint32ToBytes(high), name.getBytes(StandardCharsets.UTF_8));
		return digestToUint64(digest) | NAMESPACE_FLAG;
	}

	/**
	 * Generates a namespace id with no parent.
	 *
	 * @param name Namespace name.
	 * @return Computed namespace id.
	 */
	public static long generateNamespaceId(final String name) {
		return generateNamespaceId(name, 0L);
	}

	/**
	 * Determines if {@code mosaicId} is an alias.
	 *
	 * @param mosaicId Mosaic id to check.
	 * @return {@code true} if the specified mosaic id is an alias.
	 */
	public static boolean isMosaicAlias(final long mosaicId) {
		return 0 != (mosaicId & NAMESPACE_FLAG);
	}

	/**
	 * Returns true if a name is a valid namespace name.
	 *
	 * @param name Namespace name to check.
	 * @return {@code true} if the specified name is valid.
	 */
	public static boolean isValidNamespaceName(final String name) {
		return name != null && VALID_NAMESPACE_PATTERN.matcher(name).matches();
	}

	/**
	 * Parses a fully qualified namespace name into a path of namespace ids.
	 *
	 * @param fullyQualifiedName Fully qualified namespace name (dot-separated).
	 * @return Computed namespace path.
	 */
	public static java.util.List<Long> generateNamespacePath(final String fullyQualifiedName) {
		final java.util.List<Long> path = new java.util.ArrayList<>();
		long parentNamespaceId = 0L;
		for (String part : fullyQualifiedName.split("\\.", -1)) {
			if (!isValidNamespaceName(part))
				throw new IllegalArgumentException(
						String.format("fully qualified name is invalid due to invalid part name (%s)", fullyQualifiedName));

			parentNamespaceId = generateNamespaceId(part, parentNamespaceId);
			path.add(parentNamespaceId);
		}

		return path;
	}

	/**
	 * Generates a mosaic id from a fully qualified mosaic alias name.
	 *
	 * @param fullyQualifiedName Fully qualified mosaic name.
	 * @return Computed mosaic id.
	 */
	public static long generateMosaicAliasId(final String fullyQualifiedName) {
		final java.util.List<Long> path = generateNamespacePath(fullyQualifiedName);
		return path.get(path.size() - 1);
	}
}
