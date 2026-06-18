package org.symbol.sdk.impl;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.bouncycastle.crypto.digests.KeccakDigest;

/**
 * Ed25519 primitives based on the public-domain TweetNaCl (nacl-fast variant), pruned and modified to allow custom hash functions (SHA-512
 * for Symbol, Keccak-512 for NEM). Field elements use {@code double[16]} to faithfully preserve the original Float64Array carry semantics.
 */
public final class Tweetnacl {
	private Tweetnacl() {
	}

	/** Hash mode for ed25519 — selects the underlying hash function. */
	public enum HashMode {
		/** SHA-512 (used by the Symbol blockchain). */
		SHA2_512,
		/** Keccak-512 (used by the NEM blockchain). */
		KECCAK_512
	}

	/** Public key length in bytes. */
	public static final int CRYPTO_SIGN_PUBLICKEYBYTES = 32;
	/** Secret key length in bytes (seed || public key). */
	public static final int CRYPTO_SIGN_SECRETKEYBYTES = 64;
	/** Signature length in bytes. */
	public static final int CRYPTO_SIGN_BYTES = 64;
	/** Seed length in bytes. */
	public static final int CRYPTO_SIGN_SEEDBYTES = 32;

	// region field element constants

	/** Allocates a fresh field element. */
	public static double[] gf() {
		return new double[16];
	}

	/** Allocates a field element initialized from {@code init}. */
	public static double[] gf(final double... init) {
		final double[] r = new double[16];
		for (int i = 0; i < init.length; ++i)
			r[i] = init[i];
		return r;
	}

	private static final double[] GF0 = gf();
	private static final double[] GF1 = gf(1);
	private static final double[] D = gf(0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070, 0xe898, 0x7779, 0x4079, 0x8cc7,
			0xfe73, 0x2b6f, 0x6cee, 0x5203);
	private static final double[] D2 = gf(0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0, 0xd130, 0xeef3, 0x80f2, 0x198e,
			0xfce7, 0x56df, 0xd9dc, 0x2406);
	private static final double[] X = gf(0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c, 0xdc5c, 0xfdd6, 0xe231, 0xc0a4,
			0x53fe, 0xcd6e, 0x36d3, 0x2169);
	private static final double[] Y = gf(0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666,
			0x6666, 0x6666, 0x6666, 0x6666);
	private static final double[] I = gf(0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43, 0xd7a7, 0x3dfb, 0x0099, 0x2b4d,
			0xdf0b, 0x4fc1, 0x2480, 0x2b83);

	/** L is the order of the ed25519 curve. */
	public static final double[] L = {
			0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0x10
	};

	// endregion

	// region byte-level helpers

	private static int vn(final byte[] x, final int xi, final byte[] y, final int yi, final int n) {
		int d = 0;
		for (int i = 0; i < n; ++i)
			d |= (x[xi + i] ^ y[yi + i]) & 0xFF;
		return (1 & ((d - 1) >>> 8)) - 1;
	}

	/** Constant-time comparison of two 32-byte arrays. */
	public static int cryptoVerify32(final byte[] x, final int xi, final byte[] y, final int yi) {
		return vn(x, xi, y, yi, 32);
	}

	private static void set25519(double[] r, double[] a) {
		for (int i = 0; i < 16; ++i)
			r[i] = (long) a[i];
	}

	private static void car25519(final double[] o) {
		double c = 1;
		for (int i = 0; i < 16; ++i) {
			final double v = o[i] + c + 65535;
			c = Math.floor(v / 65536);
			o[i] = v - c * 65536;
		}
		o[0] += c - 1 + 37 * (c - 1);
	}

	private static void sel25519(final double[] p, final double[] q, final int b) {
		final long c = ~((long) b - 1);
		for (int i = 0; i < 16; ++i) {
			final long t = c & ((long) p[i] ^ (long) q[i]);
			p[i] = (long) p[i] ^ t;
			q[i] = (long) q[i] ^ t;
		}
	}

	private static void pack25519(final byte[] o, final double[] n) {
		final double[] m = gf();
		final double[] t = gf();
		for (int i = 0; i < 16; ++i)
			t[i] = n[i];
		car25519(t);
		car25519(t);
		car25519(t);
		for (int j = 0; j < 2; ++j) {
			m[0] = t[0] - 0xffed;
			for (int i = 1; i < 15; ++i) {
				m[i] = t[i] - 0xffff - (((long) m[i - 1] >> 16) & 1);
				m[i - 1] = ((long) m[i - 1]) & 0xffff;
			}
			m[15] = t[15] - 0x7fff - (((long) m[14] >> 16) & 1);
			final int b = (int) (((long) m[15] >> 16) & 1);
			m[14] = ((long) m[14]) & 0xffff;
			sel25519(t, m, 1 - b);
		}
		for (int i = 0; i < 16; ++i) {
			o[2 * i] = (byte) (((long) t[i]) & 0xff);
			o[2 * i + 1] = (byte) (((long) t[i]) >> 8);
		}
	}

	/** Returns 0 if the two field elements pack to identical 32-byte sequences. */
	public static int neq25519(double[] a, double[] b) {
		final byte[] c = new byte[32];
		final byte[] d = new byte[32];
		pack25519(c, a);
		pack25519(d, b);
		return cryptoVerify32(c, 0, d, 0);
	}

	private static int par25519(double[] a) {
		final byte[] d = new byte[32];
		pack25519(d, a);
		return d[0] & 1;
	}

	private static void unpack25519(final double[] o, final byte[] n) {
		for (int i = 0; i < 16; ++i)
			o[i] = (n[2 * i] & 0xff) + ((n[2 * i + 1] & 0xff) << 8);
		o[15] = ((long) o[15]) & 0x7fff;
	}

	private static void fieldAdd(double[] o, double[] a, double[] b) {
		for (int i = 0; i < 16; ++i)
			o[i] = a[i] + b[i];
	}

	/** Field subtraction: o = a - b. Public for use by SharedKey/scalarmult helpers. */
	public static void zSub(double[] o, double[] a, double[] b) {
		for (int i = 0; i < 16; ++i)
			o[i] = a[i] - b[i];
	}

	private static void fieldMul(double[] o, double[] a, double[] b) {
		double t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0, t7 = 0;
		double t8 = 0, t9 = 0, t10 = 0, t11 = 0, t12 = 0, t13 = 0, t14 = 0, t15 = 0;
		double t16 = 0, t17 = 0, t18 = 0, t19 = 0, t20 = 0, t21 = 0, t22 = 0, t23 = 0;
		double t24 = 0, t25 = 0, t26 = 0, t27 = 0, t28 = 0, t29 = 0, t30 = 0;
		final double b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3], b4 = b[4], b5 = b[5], b6 = b[6], b7 = b[7];
		final double b8 = b[8], b9 = b[9], b10 = b[10], b11 = b[11], b12 = b[12], b13 = b[13], b14 = b[14], b15 = b[15];

		double v;
		v = a[0];
		t0 += v * b0;
		t1 += v * b1;
		t2 += v * b2;
		t3 += v * b3;
		t4 += v * b4;
		t5 += v * b5;
		t6 += v * b6;
		t7 += v * b7;
		t8 += v * b8;
		t9 += v * b9;
		t10 += v * b10;
		t11 += v * b11;
		t12 += v * b12;
		t13 += v * b13;
		t14 += v * b14;
		t15 += v * b15;
		v = a[1];
		t1 += v * b0;
		t2 += v * b1;
		t3 += v * b2;
		t4 += v * b3;
		t5 += v * b4;
		t6 += v * b5;
		t7 += v * b6;
		t8 += v * b7;
		t9 += v * b8;
		t10 += v * b9;
		t11 += v * b10;
		t12 += v * b11;
		t13 += v * b12;
		t14 += v * b13;
		t15 += v * b14;
		t16 += v * b15;
		v = a[2];
		t2 += v * b0;
		t3 += v * b1;
		t4 += v * b2;
		t5 += v * b3;
		t6 += v * b4;
		t7 += v * b5;
		t8 += v * b6;
		t9 += v * b7;
		t10 += v * b8;
		t11 += v * b9;
		t12 += v * b10;
		t13 += v * b11;
		t14 += v * b12;
		t15 += v * b13;
		t16 += v * b14;
		t17 += v * b15;
		v = a[3];
		t3 += v * b0;
		t4 += v * b1;
		t5 += v * b2;
		t6 += v * b3;
		t7 += v * b4;
		t8 += v * b5;
		t9 += v * b6;
		t10 += v * b7;
		t11 += v * b8;
		t12 += v * b9;
		t13 += v * b10;
		t14 += v * b11;
		t15 += v * b12;
		t16 += v * b13;
		t17 += v * b14;
		t18 += v * b15;
		v = a[4];
		t4 += v * b0;
		t5 += v * b1;
		t6 += v * b2;
		t7 += v * b3;
		t8 += v * b4;
		t9 += v * b5;
		t10 += v * b6;
		t11 += v * b7;
		t12 += v * b8;
		t13 += v * b9;
		t14 += v * b10;
		t15 += v * b11;
		t16 += v * b12;
		t17 += v * b13;
		t18 += v * b14;
		t19 += v * b15;
		v = a[5];
		t5 += v * b0;
		t6 += v * b1;
		t7 += v * b2;
		t8 += v * b3;
		t9 += v * b4;
		t10 += v * b5;
		t11 += v * b6;
		t12 += v * b7;
		t13 += v * b8;
		t14 += v * b9;
		t15 += v * b10;
		t16 += v * b11;
		t17 += v * b12;
		t18 += v * b13;
		t19 += v * b14;
		t20 += v * b15;
		v = a[6];
		t6 += v * b0;
		t7 += v * b1;
		t8 += v * b2;
		t9 += v * b3;
		t10 += v * b4;
		t11 += v * b5;
		t12 += v * b6;
		t13 += v * b7;
		t14 += v * b8;
		t15 += v * b9;
		t16 += v * b10;
		t17 += v * b11;
		t18 += v * b12;
		t19 += v * b13;
		t20 += v * b14;
		t21 += v * b15;
		v = a[7];
		t7 += v * b0;
		t8 += v * b1;
		t9 += v * b2;
		t10 += v * b3;
		t11 += v * b4;
		t12 += v * b5;
		t13 += v * b6;
		t14 += v * b7;
		t15 += v * b8;
		t16 += v * b9;
		t17 += v * b10;
		t18 += v * b11;
		t19 += v * b12;
		t20 += v * b13;
		t21 += v * b14;
		t22 += v * b15;
		v = a[8];
		t8 += v * b0;
		t9 += v * b1;
		t10 += v * b2;
		t11 += v * b3;
		t12 += v * b4;
		t13 += v * b5;
		t14 += v * b6;
		t15 += v * b7;
		t16 += v * b8;
		t17 += v * b9;
		t18 += v * b10;
		t19 += v * b11;
		t20 += v * b12;
		t21 += v * b13;
		t22 += v * b14;
		t23 += v * b15;
		v = a[9];
		t9 += v * b0;
		t10 += v * b1;
		t11 += v * b2;
		t12 += v * b3;
		t13 += v * b4;
		t14 += v * b5;
		t15 += v * b6;
		t16 += v * b7;
		t17 += v * b8;
		t18 += v * b9;
		t19 += v * b10;
		t20 += v * b11;
		t21 += v * b12;
		t22 += v * b13;
		t23 += v * b14;
		t24 += v * b15;
		v = a[10];
		t10 += v * b0;
		t11 += v * b1;
		t12 += v * b2;
		t13 += v * b3;
		t14 += v * b4;
		t15 += v * b5;
		t16 += v * b6;
		t17 += v * b7;
		t18 += v * b8;
		t19 += v * b9;
		t20 += v * b10;
		t21 += v * b11;
		t22 += v * b12;
		t23 += v * b13;
		t24 += v * b14;
		t25 += v * b15;
		v = a[11];
		t11 += v * b0;
		t12 += v * b1;
		t13 += v * b2;
		t14 += v * b3;
		t15 += v * b4;
		t16 += v * b5;
		t17 += v * b6;
		t18 += v * b7;
		t19 += v * b8;
		t20 += v * b9;
		t21 += v * b10;
		t22 += v * b11;
		t23 += v * b12;
		t24 += v * b13;
		t25 += v * b14;
		t26 += v * b15;
		v = a[12];
		t12 += v * b0;
		t13 += v * b1;
		t14 += v * b2;
		t15 += v * b3;
		t16 += v * b4;
		t17 += v * b5;
		t18 += v * b6;
		t19 += v * b7;
		t20 += v * b8;
		t21 += v * b9;
		t22 += v * b10;
		t23 += v * b11;
		t24 += v * b12;
		t25 += v * b13;
		t26 += v * b14;
		t27 += v * b15;
		v = a[13];
		t13 += v * b0;
		t14 += v * b1;
		t15 += v * b2;
		t16 += v * b3;
		t17 += v * b4;
		t18 += v * b5;
		t19 += v * b6;
		t20 += v * b7;
		t21 += v * b8;
		t22 += v * b9;
		t23 += v * b10;
		t24 += v * b11;
		t25 += v * b12;
		t26 += v * b13;
		t27 += v * b14;
		t28 += v * b15;
		v = a[14];
		t14 += v * b0;
		t15 += v * b1;
		t16 += v * b2;
		t17 += v * b3;
		t18 += v * b4;
		t19 += v * b5;
		t20 += v * b6;
		t21 += v * b7;
		t22 += v * b8;
		t23 += v * b9;
		t24 += v * b10;
		t25 += v * b11;
		t26 += v * b12;
		t27 += v * b13;
		t28 += v * b14;
		t29 += v * b15;
		v = a[15];
		t15 += v * b0;
		t16 += v * b1;
		t17 += v * b2;
		t18 += v * b3;
		t19 += v * b4;
		t20 += v * b5;
		t21 += v * b6;
		t22 += v * b7;
		t23 += v * b8;
		t24 += v * b9;
		t25 += v * b10;
		t26 += v * b11;
		t27 += v * b12;
		t28 += v * b13;
		t29 += v * b14;
		t30 += v * b15;

		t0 += 38 * t16;
		t1 += 38 * t17;
		t2 += 38 * t18;
		t3 += 38 * t19;
		t4 += 38 * t20;
		t5 += 38 * t21;
		t6 += 38 * t22;
		t7 += 38 * t23;
		t8 += 38 * t24;
		t9 += 38 * t25;
		t10 += 38 * t26;
		t11 += 38 * t27;
		t12 += 38 * t28;
		t13 += 38 * t29;
		t14 += 38 * t30;
		// t15 left as is

		// first car
		double c = 1;
		double vv;
		vv = t0 + c + 65535;
		c = Math.floor(vv / 65536);
		t0 = vv - c * 65536;
		vv = t1 + c + 65535;
		c = Math.floor(vv / 65536);
		t1 = vv - c * 65536;
		vv = t2 + c + 65535;
		c = Math.floor(vv / 65536);
		t2 = vv - c * 65536;
		vv = t3 + c + 65535;
		c = Math.floor(vv / 65536);
		t3 = vv - c * 65536;
		vv = t4 + c + 65535;
		c = Math.floor(vv / 65536);
		t4 = vv - c * 65536;
		vv = t5 + c + 65535;
		c = Math.floor(vv / 65536);
		t5 = vv - c * 65536;
		vv = t6 + c + 65535;
		c = Math.floor(vv / 65536);
		t6 = vv - c * 65536;
		vv = t7 + c + 65535;
		c = Math.floor(vv / 65536);
		t7 = vv - c * 65536;
		vv = t8 + c + 65535;
		c = Math.floor(vv / 65536);
		t8 = vv - c * 65536;
		vv = t9 + c + 65535;
		c = Math.floor(vv / 65536);
		t9 = vv - c * 65536;
		vv = t10 + c + 65535;
		c = Math.floor(vv / 65536);
		t10 = vv - c * 65536;
		vv = t11 + c + 65535;
		c = Math.floor(vv / 65536);
		t11 = vv - c * 65536;
		vv = t12 + c + 65535;
		c = Math.floor(vv / 65536);
		t12 = vv - c * 65536;
		vv = t13 + c + 65535;
		c = Math.floor(vv / 65536);
		t13 = vv - c * 65536;
		vv = t14 + c + 65535;
		c = Math.floor(vv / 65536);
		t14 = vv - c * 65536;
		vv = t15 + c + 65535;
		c = Math.floor(vv / 65536);
		t15 = vv - c * 65536;
		t0 += c - 1 + 37 * (c - 1);

		// second car
		c = 1;
		vv = t0 + c + 65535;
		c = Math.floor(vv / 65536);
		t0 = vv - c * 65536;
		vv = t1 + c + 65535;
		c = Math.floor(vv / 65536);
		t1 = vv - c * 65536;
		vv = t2 + c + 65535;
		c = Math.floor(vv / 65536);
		t2 = vv - c * 65536;
		vv = t3 + c + 65535;
		c = Math.floor(vv / 65536);
		t3 = vv - c * 65536;
		vv = t4 + c + 65535;
		c = Math.floor(vv / 65536);
		t4 = vv - c * 65536;
		vv = t5 + c + 65535;
		c = Math.floor(vv / 65536);
		t5 = vv - c * 65536;
		vv = t6 + c + 65535;
		c = Math.floor(vv / 65536);
		t6 = vv - c * 65536;
		vv = t7 + c + 65535;
		c = Math.floor(vv / 65536);
		t7 = vv - c * 65536;
		vv = t8 + c + 65535;
		c = Math.floor(vv / 65536);
		t8 = vv - c * 65536;
		vv = t9 + c + 65535;
		c = Math.floor(vv / 65536);
		t9 = vv - c * 65536;
		vv = t10 + c + 65535;
		c = Math.floor(vv / 65536);
		t10 = vv - c * 65536;
		vv = t11 + c + 65535;
		c = Math.floor(vv / 65536);
		t11 = vv - c * 65536;
		vv = t12 + c + 65535;
		c = Math.floor(vv / 65536);
		t12 = vv - c * 65536;
		vv = t13 + c + 65535;
		c = Math.floor(vv / 65536);
		t13 = vv - c * 65536;
		vv = t14 + c + 65535;
		c = Math.floor(vv / 65536);
		t14 = vv - c * 65536;
		vv = t15 + c + 65535;
		c = Math.floor(vv / 65536);
		t15 = vv - c * 65536;
		t0 += c - 1 + 37 * (c - 1);

		o[0] = t0;
		o[1] = t1;
		o[2] = t2;
		o[3] = t3;
		o[4] = t4;
		o[5] = t5;
		o[6] = t6;
		o[7] = t7;
		o[8] = t8;
		o[9] = t9;
		o[10] = t10;
		o[11] = t11;
		o[12] = t12;
		o[13] = t13;
		o[14] = t14;
		o[15] = t15;
	}

	private static void fieldSqr(double[] o, double[] a) {
		fieldMul(o, a, a);
	}

	private static void inv25519(double[] o, double[] i) {
		final double[] c = gf();
		for (int a = 0; a < 16; ++a)
			c[a] = i[a];
		for (int a = 253; 0 <= a; --a) {
			fieldSqr(c, c);
			if (2 != a && 4 != a)
				fieldMul(c, c, i);
		}
		for (int a = 0; a < 16; ++a)
			o[a] = c[a];
	}

	private static void pow2523(double[] o, double[] i) {
		final double[] c = gf();
		for (int a = 0; a < 16; ++a)
			c[a] = i[a];
		for (int a = 250; 0 <= a; --a) {
			fieldSqr(c, c);
			if (1 != a)
				fieldMul(c, c, i);
		}
		for (int a = 0; a < 16; ++a)
			o[a] = c[a];
	}

	// endregion

	// region hashing

	private static MessageDigest createSha2_512() {
		try {
			return MessageDigest.getInstance("SHA-512");
		} catch (NoSuchAlgorithmException ex) {
			throw new IllegalStateException(ex);
		}
	}

	/**
	 * Computes the hash of the first {@code n} bytes of {@code m} using the chosen mode and writes the result into the start of
	 * {@code out}.
	 *
	 * @param out Output buffer (only the first 64 bytes are written).
	 * @param m Message buffer.
	 * @param n Number of message bytes to hash.
	 * @param mode Hash mode.
	 */
	public static void cryptoHash(final byte[] out, final byte[] m, final int n, final HashMode mode) {
		cryptoHash(out, m, 0, n, mode);
	}

	private static void cryptoHash(final byte[] out, final byte[] m, final int offset, final int n, final HashMode mode) {
		byte[] hash;
		if (HashMode.KECCAK_512 == mode) {
			final KeccakDigest digest = new KeccakDigest(512);
			digest.update(m, offset, n);
			hash = new byte[64];
			digest.doFinal(hash, 0);
		} else {
			final MessageDigest digest = createSha2_512();
			digest.update(m, offset, n);
			hash = digest.digest();
		}

		System.arraycopy(hash, 0, out, 0, Math.min(out.length, hash.length));
	}

	// endregion

	// region group operations

	private static void edwardsAdd(final double[][] p, final double[][] q) {
		final double[] a = gf(), b = gf(), c = gf();
		final double[] d = gf(), e = gf(), f = gf();
		final double[] g = gf(), h = gf(), t = gf();

		zSub(a, p[1], p[0]);
		zSub(t, q[1], q[0]);
		fieldMul(a, a, t);
		fieldAdd(b, p[0], p[1]);
		fieldAdd(t, q[0], q[1]);
		fieldMul(b, b, t);
		fieldMul(c, p[3], q[3]);
		fieldMul(c, c, D2);
		fieldMul(d, p[2], q[2]);
		fieldAdd(d, d, d);
		zSub(e, b, a);
		zSub(f, d, c);
		fieldAdd(g, d, c);
		fieldAdd(h, b, a);

		fieldMul(p[0], e, f);
		fieldMul(p[1], h, g);
		fieldMul(p[2], g, f);
		fieldMul(p[3], e, h);
	}

	private static void cswap(final double[][] p, final double[][] q, final int b) {
		for (int i = 0; i < 4; ++i)
			sel25519(p[i], q[i], b);
	}

	/** Packs an extended Edwards point into a 32-byte encoding. */
	public static void pack(final byte[] r, final double[][] p) {
		final double[] tx = gf(), ty = gf(), zi = gf();
		inv25519(zi, p[2]);
		fieldMul(tx, p[0], zi);
		fieldMul(ty, p[1], zi);
		pack25519(r, ty);
		r[31] ^= (byte) (par25519(tx) << 7);
	}

	/** Computes p = q^s on the curve, with s a little-endian byte string scalar. */
	public static void scalarmult(final double[][] p, final double[][] q, final byte[] s) {
		set25519(p[0], GF0);
		set25519(p[1], GF1);
		set25519(p[2], GF1);
		set25519(p[3], GF0);
		for (int i = 255; 0 <= i; --i) {
			final int b = ((s[(i / 8)] & 0xFF) >> (i & 7)) & 1;
			cswap(p, q, b);
			edwardsAdd(q, p);
			edwardsAdd(p, p);
			cswap(p, q, b);
		}
	}

	/** Variant of scalarmult that accepts a {@code double[]} scalar (used by L-multiplication). */
	public static void scalarmult(final double[][] p, final double[][] q, final double[] s) {
		final byte[] sb = new byte[32];
		for (int i = 0; i < 32; ++i)
			sb[i] = (byte) ((long) s[i] & 0xFF);
		scalarmult(p, q, sb);
	}

	private static void scalarbase(final double[][] p, final byte[] s) {
		final double[][] q = {
				gf(), gf(), gf(), gf()
		};
		set25519(q[0], X);
		set25519(q[1], Y);
		set25519(q[2], GF1);
		fieldMul(q[3], X, Y);
		scalarmult(p, q, s);
	}

	private static int cryptoSignKeypair(final byte[] pk, final byte[] sk, final HashMode mode) {
		final byte[] d = new byte[64];
		final double[][] p = {
				gf(), gf(), gf(), gf()
		};

		cryptoHash(d, sk, 32, mode);
		d[0] = (byte) (d[0] & 248);
		d[31] = (byte) (d[31] & 127);
		d[31] = (byte) (d[31] | 64);

		scalarbase(p, d);
		pack(pk, p);

		System.arraycopy(pk, 0, sk, 32, 32);
		return 0;
	}

	/**
	 * Reduces {@code x} modulo L and writes the result into {@code r} starting at {@code rOffset}.
	 *
	 * @param r Output buffer (32 bytes are written).
	 * @param rOffset Offset within {@code r} to start writing.
	 * @param x Input scratch (length &gt;= 64). This is mutated.
	 */
	public static void modL(final byte[] r, final int rOffset, final double[] x) {
		double carry;
		int j = 0;
		for (int i = 63; 32 <= i; --i) {
			carry = 0;
			final int k = i - 12;
			for (j = i - 32; j < k; ++j) {
				x[j] += carry - 16 * x[i] * L[j - (i - 32)];
				carry = Math.floor((x[j] + 128) / 256);
				x[j] -= carry * 256;
			}
			x[j] += carry;
			x[i] = 0;
		}
		carry = 0;
		for (j = 0; j < 32; ++j) {
			x[j] += carry - ((long) x[31] >> 4) * L[j];
			carry = (long) x[j] >> 8;
			x[j] = ((long) x[j]) & 255;
		}
		for (j = 0; j < 32; ++j)
			x[j] -= carry * L[j];
		for (int i = 0; i < 32; ++i) {
			x[i + 1] += (long) x[i] >> 8;
			r[rOffset + i] = (byte) (((long) x[i]) & 255);
		}
	}

	private static void reduce(final byte[] r) {
		final double[] x = new double[64];
		for (int i = 0; i < 64; ++i)
			x[i] = r[i] & 0xFF;
		for (int i = 0; i < 64; ++i)
			r[i] = 0;
		modL(r, 0, x);
	}

	/** Returns the length of the resulting signed message. */
	private static int cryptoSign(final byte[] sm, final byte[] m, final int n, final byte[] sk, final HashMode mode) {
		final byte[] d = new byte[64];
		final byte[] h = new byte[64];
		final byte[] r = new byte[64];
		final double[] x = new double[64];
		final double[][] p = {
				gf(), gf(), gf(), gf()
		};

		cryptoHash(d, sk, 32, mode);
		d[0] = (byte) (d[0] & 248);
		d[31] = (byte) (d[31] & 127);
		d[31] = (byte) (d[31] | 64);

		final int smlen = n + 64;
		System.arraycopy(m, 0, sm, 64, n);
		System.arraycopy(d, 32, sm, 32, 32);

		// hash sm[32 .. 32+n+32] == d[32..64] || m
		cryptoHash(r, sm, 32, n + 32, mode);
		reduce(r);
		scalarbase(p, r);
		pack(sm, p);

		System.arraycopy(sk, 32, sm, 32, 32);
		cryptoHash(h, sm, 0, n + 64, mode);
		reduce(h);

		for (int i = 0; i < 64; ++i)
			x[i] = 0;
		for (int i = 0; i < 32; ++i)
			x[i] = r[i] & 0xFF;
		for (int i = 0; i < 32; ++i) {
			for (int jj = 0; jj < 32; ++jj)
				x[i + jj] += (h[i] & 0xFF) * (d[jj] & 0xFF);
		}

		modL(sm, 32, x);
		return smlen;
	}

	/** Unpacks an encoded point and negates it (returns -1 on failure, 0 on success). */
	public static int unpackneg(final double[][] r, final byte[] p) {
		final double[] t = gf(), chk = gf(), num = gf();
		final double[] den = gf(), den2 = gf(), den4 = gf();
		final double[] den6 = gf();

		set25519(r[2], GF1);
		unpack25519(r[1], p);
		fieldSqr(num, r[1]);
		fieldMul(den, num, D);
		zSub(num, num, r[2]);
		fieldAdd(den, r[2], den);

		fieldSqr(den2, den);
		fieldSqr(den4, den2);
		fieldMul(den6, den4, den2);
		fieldMul(t, den6, num);
		fieldMul(t, t, den);

		pow2523(t, t);
		fieldMul(t, t, num);
		fieldMul(t, t, den);
		fieldMul(t, t, den);
		fieldMul(r[0], t, den);

		fieldSqr(chk, r[0]);
		fieldMul(chk, chk, den);
		if (0 != neq25519(chk, num))
			fieldMul(r[0], r[0], I);

		fieldSqr(chk, r[0]);
		fieldMul(chk, chk, den);
		if (0 != neq25519(chk, num))
			return -1;

		if (par25519(r[0]) == ((p[31] & 0xFF) >> 7))
			zSub(r[0], GF0, r[0]);

		fieldMul(r[3], r[0], r[1]);
		return 0;
	}

	private static int cryptoSignOpen(final byte[] m, final byte[] sm, final int n, final byte[] pk, final HashMode mode) {
		final byte[] t = new byte[32];
		final byte[] h = new byte[64];
		final double[][] p = {
				gf(), gf(), gf(), gf()
		};
		final double[][] q = {
				gf(), gf(), gf(), gf()
		};

		if (64 > n)
			return -1;

		if (0 != unpackneg(q, pk))
			return -1;

		System.arraycopy(sm, 0, m, 0, n);
		System.arraycopy(pk, 0, m, 32, 32);
		cryptoHash(h, m, n, mode);
		reduce(h);
		scalarmult(p, q, h);

		// scalarbase from sm[32..]
		final byte[] smTail = new byte[32];
		System.arraycopy(sm, 32, smTail, 0, 32);
		scalarbase(q, smTail);
		edwardsAdd(p, q);
		pack(t, p);

		final int newN = n - 64;
		if (0 != cryptoVerify32(sm, 0, t, 0)) {
			for (int i = 0; i < newN; ++i)
				m[i] = 0;
			return -1;
		}

		for (int i = 0; i < newN; ++i)
			m[i] = sm[i + 64];
		return newN;
	}

	// endregion

	// region high-level API

	/** A signing keypair. */
	public static final class KeyPair {
		/** Public key bytes (32). */
		public final byte[] publicKey;
		/** Secret key bytes (64 = seed || publicKey). */
		public final byte[] secretKey;

		KeyPair(final byte[] publicKey, final byte[] secretKey) {
			this.publicKey = publicKey;
			this.secretKey = secretKey;
		}
	}

	/**
	 * Builds a keypair from a 32-byte seed.
	 *
	 * @param seed Seed bytes (length must be 32).
	 * @param mode Hash mode.
	 * @return Keypair derived from the seed.
	 */
	public static KeyPair signKeyPairFromSeed(final byte[] seed, final HashMode mode) {
		if (CRYPTO_SIGN_SEEDBYTES != seed.length)
			throw new IllegalArgumentException("bad seed size");

		final byte[] pk = new byte[CRYPTO_SIGN_PUBLICKEYBYTES];
		final byte[] sk = new byte[CRYPTO_SIGN_SECRETKEYBYTES];
		System.arraycopy(seed, 0, sk, 0, 32);
		cryptoSignKeypair(pk, sk, mode);
		return new KeyPair(pk, sk);
	}

	/**
	 * Computes a detached Ed25519 signature.
	 *
	 * @param msg Message bytes.
	 * @param secretKey Secret key bytes (length must be 64).
	 * @param mode Hash mode.
	 * @return Detached signature (64 bytes).
	 */
	public static byte[] signDetached(final byte[] msg, final byte[] secretKey, final HashMode mode) {
		if (CRYPTO_SIGN_SECRETKEYBYTES != secretKey.length)
			throw new IllegalArgumentException("bad secret key size");

		final byte[] signedMsg = new byte[CRYPTO_SIGN_BYTES + msg.length];
		cryptoSign(signedMsg, msg, msg.length, secretKey, mode);
		final byte[] sig = new byte[CRYPTO_SIGN_BYTES];
		System.arraycopy(signedMsg, 0, sig, 0, CRYPTO_SIGN_BYTES);
		return sig;
	}

	/**
	 * Verifies a detached Ed25519 signature.
	 *
	 * @param msg Message bytes.
	 * @param sig Signature bytes (length must be 64).
	 * @param publicKey Public key bytes (length must be 32).
	 * @param mode Hash mode.
	 * @return {@code true} if the signature is valid.
	 */
	public static boolean signDetachedVerify(final byte[] msg, final byte[] sig, final byte[] publicKey, final HashMode mode) {
		if (CRYPTO_SIGN_BYTES != sig.length)
			throw new IllegalArgumentException("bad signature size");

		if (CRYPTO_SIGN_PUBLICKEYBYTES != publicKey.length)
			throw new IllegalArgumentException("bad public key size");

		final byte[] sm = new byte[CRYPTO_SIGN_BYTES + msg.length];
		final byte[] m = new byte[CRYPTO_SIGN_BYTES + msg.length];
		System.arraycopy(sig, 0, sm, 0, CRYPTO_SIGN_BYTES);
		System.arraycopy(msg, 0, sm, CRYPTO_SIGN_BYTES, msg.length);
		return 0 <= cryptoSignOpen(m, sm, sm.length, publicKey, mode);
	}

	// endregion
}
