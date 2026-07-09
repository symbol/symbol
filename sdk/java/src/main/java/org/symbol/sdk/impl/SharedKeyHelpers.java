package org.symbol.sdk.impl;

import java.nio.charset.StandardCharsets;
import java.util.function.BiFunction;

import org.bouncycastle.crypto.digests.SHA256Digest;
import org.bouncycastle.crypto.generators.HKDFBytesGenerator;
import org.bouncycastle.crypto.params.HKDFParameters;

import org.symbol.sdk.CryptoTypes;

/**
 * Factories for deriving shared secrets and shared keys between an Ed25519 keypair and another party's public key; supports both Symbol
 * (SHA-512) and NEM (Keccak-512) via {@link Tweetnacl.HashMode}.
 */
public final class SharedKeyHelpers {
	private SharedKeyHelpers() {
	}

	/**
	 * Factory that produces a shared 32-byte secret from a private key (raw bytes) and another party's public key.
	 */
	@FunctionalInterface
	public interface SharedSecretDeriver {
		/**
		 * Derives a shared secret.
		 *
		 * @param privateKeyBytes Private key bytes (32).
		 * @param otherPublicKey Other party's public key.
		 * @return 32-byte shared secret.
		 */
		byte[] derive(byte[] privateKeyBytes, CryptoTypes.PublicKey otherPublicKey);
	}

	/**
	 * Factory that produces a {@link CryptoTypes.SharedKey256} from a private key (raw bytes) and another party's public key.
	 */
	@FunctionalInterface
	public interface SharedKeyDeriver {
		/**
		 * Derives a shared key.
		 *
		 * @param privateKeyBytes Private key bytes (32).
		 * @param otherPublicKey Other party's public key.
		 * @return Derived shared key.
		 */
		CryptoTypes.SharedKey256 derive(byte[] privateKeyBytes, CryptoTypes.PublicKey otherPublicKey);
	}

	// publicKey is canonical if the y coordinate is smaller than 2^255 - 19
	// note: this version is based on server version and should be constant-time
	private static boolean isCanonicalKey(final CryptoTypes.PublicKey publicKey) {
		final byte[] buffer = publicKey.bytes();
		int a = ((buffer[31] & 0xFF) & 0x7F) ^ 0x7F;
		for (int i = 30; 0 < i; --i)
			a |= (buffer[i] & 0xFF) ^ 0xFF;

		a = (a - 1) >>> 8;

		final int b = (0xED - 1 - (buffer[0] & 0xFF)) >>> 8;
		return 0 != 1 - (a & b & 1);
	}

	private static boolean isInMainSubgroup(final double[][] point) {
		final double[][] result = {
				Tweetnacl.gf(), Tweetnacl.gf(), Tweetnacl.gf(), Tweetnacl.gf()
		};
		// multiply by group order
		Tweetnacl.scalarmult(result, point, Tweetnacl.L);

		// check if result is neutral element
		final double[] gf0 = Tweetnacl.gf();
		final int areEqual = Tweetnacl.neq25519(result[1], result[2]);
		final int isZero = Tweetnacl.neq25519(gf0, result[0]);

		// yes, this is supposed to be bit OR — keeps the check constant-time (no short-circuit)
		return 0 == (areEqual | isZero);
	}

	/**
	 * Creates a shared secret factory for a given hash mode.
	 *
	 * @param mode Hash mode used when hashing the private key (Sha2_512 for Symbol, Keccak_512 for NEM).
	 * @return Function that derives a 32-byte shared secret from a private key and the other party's public key.
	 */
	public static SharedSecretDeriver deriveSharedSecretFactory(final Tweetnacl.HashMode mode) {
		return (privateKeyBytes, otherPublicKey) -> {
			final double[][] point = {
					Tweetnacl.gf(), Tweetnacl.gf(), Tweetnacl.gf(), Tweetnacl.gf()
			};

			if (!isCanonicalKey(otherPublicKey) || 0 != Tweetnacl.unpackneg(point, otherPublicKey.bytes()) || !isInMainSubgroup(point))
				throw new IllegalArgumentException("invalid point");

			// negate point == negate X coordinate and 't'
			final double[] gf0 = Tweetnacl.gf();
			Tweetnacl.zSub(point[0], gf0, point[0]);
			Tweetnacl.zSub(point[3], gf0, point[3]);

			final byte[] scalar = new byte[64];
			Tweetnacl.cryptoHash(scalar, privateKeyBytes, 32, mode);
			scalar[0] = (byte) (scalar[0] & 248);
			scalar[31] = (byte) (scalar[31] & 127);
			scalar[31] = (byte) (scalar[31] | 64);

			final double[][] result = {
					Tweetnacl.gf(), Tweetnacl.gf(), Tweetnacl.gf(), Tweetnacl.gf()
			};
			Tweetnacl.scalarmult(result, point, scalar);

			final byte[] sharedSecret = new byte[32];
			Tweetnacl.pack(sharedSecret, result);
			return sharedSecret;
		};
	}

	/**
	 * Creates a shared key factory for a given HKDF info tag and hash mode.
	 *
	 * @param info Tag used in the HKDF algorithm ({@code "catapult"} for Symbol, {@code "nem-nis1"} for NEM).
	 * @param mode Hash mode used in the underlying shared-secret derivation.
	 * @return Function that derives a {@link CryptoTypes.SharedKey256} from a private key and the other party's public key.
	 */
	public static SharedKeyDeriver deriveSharedKeyFactory(final String info, final Tweetnacl.HashMode mode) {
		final SharedSecretDeriver secretDeriver = deriveSharedSecretFactory(mode);
		final BiFunction<byte[], byte[], byte[]> hkdf = (sharedSecret, infoBytes) -> {
			final HKDFBytesGenerator gen = new HKDFBytesGenerator(new SHA256Digest());
			// HKDF-SHA-256 with an empty salt, "catapult"/"nem-nis1" info, 32-byte output.
			gen.init(new HKDFParameters(sharedSecret, null, infoBytes));
			final byte[] out = new byte[32];
			gen.generateBytes(out, 0, 32);
			return out;
		};
		final byte[] infoBytes = info.getBytes(StandardCharsets.UTF_8);
		return (privateKeyBytes, otherPublicKey) -> {
			final byte[] sharedSecret = secretDeriver.derive(privateKeyBytes, otherPublicKey);
			return new CryptoTypes.SharedKey256(hkdf.apply(sharedSecret, infoBytes));
		};
	}
}
