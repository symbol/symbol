package org.symbol.sdk.utils;

import org.bouncycastle.crypto.digests.KeccakDigest;
import org.bouncycastle.crypto.digests.RIPEMD160Digest;
import org.bouncycastle.crypto.digests.SHA3Digest;

/**
 * Cryptographic transform helpers.
 */
public final class Transforms {
	private Transforms() {
	}

	/**
	 * Hashes the concatenation of {@code first} and {@code second} with SHA3-256. Fixed-arity overload that avoids the {@code byte[][]}
	 * varargs allocation on the common two-part hashing paths (merkle node pairs, leaf hashes).
	 *
	 * @param first First input buffer.
	 * @param second Second input buffer.
	 * @return 32-byte SHA3-256 digest.
	 */
	public static byte[] sha3_256(final byte[] first, final byte[] second) {
		// fixed-arity twin of the varargs sha3_256 below — must hash identically (same digest, same in-order
		// updates); keep the two in sync if the hashing ever changes (e.g. domain separation / length prefixing).
		final SHA3Digest hasher = new SHA3Digest(256);
		hasher.update(first, 0, first.length);
		hasher.update(second, 0, second.length);
		return doFinal(hasher);
	}

	/**
	 * Hashes the in-order concatenation of {@code parts} with SHA3-256.
	 *
	 * @param parts Input buffers to hash, in order.
	 * @return 32-byte SHA3-256 digest.
	 */
	public static byte[] sha3_256(final byte[]... parts) {
		final SHA3Digest hasher = new SHA3Digest(256);
		for (final byte[] part : parts)
			hasher.update(part, 0, part.length);

		return doFinal(hasher);
	}

	private static byte[] doFinal(final SHA3Digest hasher) {
		final byte[] out = new byte[hasher.getDigestSize()];
		hasher.doFinal(out, 0);
		return out;
	}

	/**
	 * Hashes payload with Keccak-256 and then hashes the result with RIPEMD-160.
	 *
	 * @param payload Input buffer to hash.
	 * @return Hash result (20 bytes).
	 */
	public static byte[] ripemdKeccak256(final byte[] payload) {
		final KeccakDigest keccak = new KeccakDigest(256);
		keccak.update(payload, 0, payload.length);
		final byte[] partOneHash = new byte[keccak.getDigestSize()];
		keccak.doFinal(partOneHash, 0);

		final RIPEMD160Digest ripemd = new RIPEMD160Digest();
		ripemd.update(partOneHash, 0, partOneHash.length);
		final byte[] result = new byte[ripemd.getDigestSize()];
		ripemd.doFinal(result, 0);
		return result;
	}
}
