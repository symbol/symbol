// this file is based on tweetnacl/nacl-fast with the following changes:
// - allows custom hash functions
// - pruned to include minimal dependencies
// - ES6

/* eslint-disable
	comma-spacing,
	computed-property-spacing,
	curly,
	func-style,
	indent,
	keyword-spacing,
	max-len,
	no-mixed-operators,
	no-mixed-spaces-and-tabs,
	no-multi-spaces,
	no-underscore-dangle,
	no-var,
	nonblock-statement-body-position,
	object-curly-spacing,
	one-var,
	one-var-declaration-per-line,
	space-before-function-paren,
	space-infix-ops,
	vars-on-top,
	yoda,
	jsdoc/require-jsdoc
*/

var gf = function(init) {
	var i, r = new Float64Array(16);
	if (init) for (i = 0; i < init.length; i++) r[i] = init[i];
	return r;
};

var _9 = new Uint8Array(32); _9[0] = 9;

var gf0 = gf(),
		gf1 = gf([1]),
		D = gf([0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070, 0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203]),
		D2 = gf([0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0, 0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406]),
		X = gf([0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c, 0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169]),
		Y = gf([0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666]),
		I = gf([0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43, 0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83]);

function vn(x, xi, y, yi, n) {
	var i,d = 0;
	for (i = 0; i < n; i++) d |= x[xi+i]^y[yi+i];
	return (1 & ((d - 1) >>> 8)) - 1;
}

function crypto_verify_32(x, xi, y, yi) {
	return vn(x,xi,y,yi,32);
}

function set25519(r, a) {
	var i;
	for (i = 0; i < 16; i++) r[i] = a[i]|0;
}

function car25519(o) {
	var i, v, c = 1;
	for (i = 0; i < 16; i++) {
		v = o[i] + c + 65535;
		c = Math.floor(v / 65536);
		o[i] = v - c * 65536;
	}
	o[0] += c-1 + 37 * (c-1);
}

function sel25519(p, q, b) {
	var t, c = ~(b-1);
	for (var i = 0; i < 16; i++) {
		t = c & (p[i] ^ q[i]);
		p[i] ^= t;
		q[i] ^= t;
	}
}

function pack25519(o, n) {
	var i, j, b;
	var m = gf(), t = gf();
	for (i = 0; i < 16; i++) t[i] = n[i];
	car25519(t);
	car25519(t);
	car25519(t);
	for (j = 0; j < 2; j++) {
		m[0] = t[0] - 0xffed;
		for (i = 1; i < 15; i++) {
			m[i] = t[i] - 0xffff - ((m[i-1]>>16) & 1);
			m[i-1] &= 0xffff;
		}
		m[15] = t[15] - 0x7fff - ((m[14]>>16) & 1);
		b = (m[15]>>16) & 1;
		m[14] &= 0xffff;
		sel25519(t, m, 1-b);
	}
	for (i = 0; i < 16; i++) {
		o[2*i] = t[i] & 0xff;
		o[2*i+1] = t[i]>>8;
	}
}

function neq25519(a, b) {
	var c = new Uint8Array(32), d = new Uint8Array(32);
	pack25519(c, a);
	pack25519(d, b);
	return crypto_verify_32(c, 0, d, 0);
}

function par25519(a) {
	var d = new Uint8Array(32);
	pack25519(d, a);
	return d[0] & 1;
}

function unpack25519(o, n) {
	var i;
	for (i = 0; i < 16; i++) o[i] = n[2*i] + (n[2*i+1] << 8);
	o[15] &= 0x7fff;
}

function A(o, a, b) {
	for (var i = 0; i < 16; i++) o[i] = a[i] + b[i];
}

function Z(o, a, b) {
	for (var i = 0; i < 16; i++) o[i] = a[i] - b[i];
}

function M(o, a, b) {
	var v, c,
		 t0 = 0,  t1 = 0,  t2 = 0,  t3 = 0,  t4 = 0,  t5 = 0,  t6 = 0,  t7 = 0,
		 t8 = 0,  t9 = 0, t10 = 0, t11 = 0, t12 = 0, t13 = 0, t14 = 0, t15 = 0,
		t16 = 0, t17 = 0, t18 = 0, t19 = 0, t20 = 0, t21 = 0, t22 = 0, t23 = 0,
		t24 = 0, t25 = 0, t26 = 0, t27 = 0, t28 = 0, t29 = 0, t30 = 0,
		b0 = b[0],
		b1 = b[1],
		b2 = b[2],
		b3 = b[3],
		b4 = b[4],
		b5 = b[5],
		b6 = b[6],
		b7 = b[7],
		b8 = b[8],
		b9 = b[9],
		b10 = b[10],
		b11 = b[11],
		b12 = b[12],
		b13 = b[13],
		b14 = b[14],
		b15 = b[15];

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

	t0  += 38 * t16;
	t1  += 38 * t17;
	t2  += 38 * t18;
	t3  += 38 * t19;
	t4  += 38 * t20;
	t5  += 38 * t21;
	t6  += 38 * t22;
	t7  += 38 * t23;
	t8  += 38 * t24;
	t9  += 38 * t25;
	t10 += 38 * t26;
	t11 += 38 * t27;
	t12 += 38 * t28;
	t13 += 38 * t29;
	t14 += 38 * t30;
	// t15 left as is

	// first car
	c = 1;
	v =  t0 + c + 65535; c = Math.floor(v / 65536);  t0 = v - c * 65536;
	v =  t1 + c + 65535; c = Math.floor(v / 65536);  t1 = v - c * 65536;
	v =  t2 + c + 65535; c = Math.floor(v / 65536);  t2 = v - c * 65536;
	v =  t3 + c + 65535; c = Math.floor(v / 65536);  t3 = v - c * 65536;
	v =  t4 + c + 65535; c = Math.floor(v / 65536);  t4 = v - c * 65536;
	v =  t5 + c + 65535; c = Math.floor(v / 65536);  t5 = v - c * 65536;
	v =  t6 + c + 65535; c = Math.floor(v / 65536);  t6 = v - c * 65536;
	v =  t7 + c + 65535; c = Math.floor(v / 65536);  t7 = v - c * 65536;
	v =  t8 + c + 65535; c = Math.floor(v / 65536);  t8 = v - c * 65536;
	v =  t9 + c + 65535; c = Math.floor(v / 65536);  t9 = v - c * 65536;
	v = t10 + c + 65535; c = Math.floor(v / 65536); t10 = v - c * 65536;
	v = t11 + c + 65535; c = Math.floor(v / 65536); t11 = v - c * 65536;
	v = t12 + c + 65535; c = Math.floor(v / 65536); t12 = v - c * 65536;
	v = t13 + c + 65535; c = Math.floor(v / 65536); t13 = v - c * 65536;
	v = t14 + c + 65535; c = Math.floor(v / 65536); t14 = v - c * 65536;
	v = t15 + c + 65535; c = Math.floor(v / 65536); t15 = v - c * 65536;
	t0 += c-1 + 37 * (c-1);

	// second car
	c = 1;
	v =  t0 + c + 65535; c = Math.floor(v / 65536);  t0 = v - c * 65536;
	v =  t1 + c + 65535; c = Math.floor(v / 65536);  t1 = v - c * 65536;
	v =  t2 + c + 65535; c = Math.floor(v / 65536);  t2 = v - c * 65536;
	v =  t3 + c + 65535; c = Math.floor(v / 65536);  t3 = v - c * 65536;
	v =  t4 + c + 65535; c = Math.floor(v / 65536);  t4 = v - c * 65536;
	v =  t5 + c + 65535; c = Math.floor(v / 65536);  t5 = v - c * 65536;
	v =  t6 + c + 65535; c = Math.floor(v / 65536);  t6 = v - c * 65536;
	v =  t7 + c + 65535; c = Math.floor(v / 65536);  t7 = v - c * 65536;
	v =  t8 + c + 65535; c = Math.floor(v / 65536);  t8 = v - c * 65536;
	v =  t9 + c + 65535; c = Math.floor(v / 65536);  t9 = v - c * 65536;
	v = t10 + c + 65535; c = Math.floor(v / 65536); t10 = v - c * 65536;
	v = t11 + c + 65535; c = Math.floor(v / 65536); t11 = v - c * 65536;
	v = t12 + c + 65535; c = Math.floor(v / 65536); t12 = v - c * 65536;
	v = t13 + c + 65535; c = Math.floor(v / 65536); t13 = v - c * 65536;
	v = t14 + c + 65535; c = Math.floor(v / 65536); t14 = v - c * 65536;
	v = t15 + c + 65535; c = Math.floor(v / 65536); t15 = v - c * 65536;
	t0 += c-1 + 37 * (c-1);

	o[ 0] = t0;
	o[ 1] = t1;
	o[ 2] = t2;
	o[ 3] = t3;
	o[ 4] = t4;
	o[ 5] = t5;
	o[ 6] = t6;
	o[ 7] = t7;
	o[ 8] = t8;
	o[ 9] = t9;
	o[10] = t10;
	o[11] = t11;
	o[12] = t12;
	o[13] = t13;
	o[14] = t14;
	o[15] = t15;
}

function S(o, a) {
	M(o, a, a);
}

function inv25519(o, i) {
	var c = gf();
	var a;
	for (a = 0; a < 16; a++) c[a] = i[a];
	for (a = 253; a >= 0; a--) {
		S(c, c);
		if(a !== 2 && a !== 4) M(c, c, i);
	}
	for (a = 0; a < 16; a++) o[a] = c[a];
}

function pow2523(o, i) {
	var c = gf();
	var a;
	for (a = 0; a < 16; a++) c[a] = i[a];
	for (a = 250; a >= 0; a--) {
			S(c, c);
			if(a !== 1) M(c, c, i);
	}
	for (a = 0; a < 16; a++) o[a] = c[a];
}

function crypto_hash(out, m, n, hasher) {
	const hashBuilder = hasher.create();
	hashBuilder.update(m.subarray(0, n));
	const hash = hashBuilder.digest();

	for (let i = 0; i < out.length; ++i)
		out[i] = hash[i];

	return 0;
}

function add(p, q) {
	var a = gf(), b = gf(), c = gf(),
			d = gf(), e = gf(), f = gf(),
			g = gf(), h = gf(), t = gf();

	Z(a, p[1], p[0]);
	Z(t, q[1], q[0]);
	M(a, a, t);
	A(b, p[0], p[1]);
	A(t, q[0], q[1]);
	M(b, b, t);
	M(c, p[3], q[3]);
	M(c, c, D2);
	M(d, p[2], q[2]);
	A(d, d, d);
	Z(e, b, a);
	Z(f, d, c);
	A(g, d, c);
	A(h, b, a);

	M(p[0], e, f);
	M(p[1], h, g);
	M(p[2], g, f);
	M(p[3], e, h);
}

function cswap(p, q, b) {
	var i;
	for (i = 0; i < 4; i++) {
		sel25519(p[i], q[i], b);
	}
}

function pack(r, p) {
	var tx = gf(), ty = gf(), zi = gf();
	inv25519(zi, p[2]);
	M(tx, p[0], zi);
	M(ty, p[1], zi);
	pack25519(r, ty);
	r[31] ^= par25519(tx) << 7;
}

function scalarmult(p, q, s) {
	var b, i;
	set25519(p[0], gf0);
	set25519(p[1], gf1);
	set25519(p[2], gf1);
	set25519(p[3], gf0);
	for (i = 255; i >= 0; --i) {
		b = (s[(i/8)|0] >> (i&7)) & 1;
		cswap(p, q, b);
		add(q, p);
		add(p, p);
		cswap(p, q, b);
	}
}

function scalarbase(p, s) {
	var q = [gf(), gf(), gf(), gf()];
	set25519(q[0], X);
	set25519(q[1], Y);
	set25519(q[2], gf1);
	M(q[3], X, Y);
	scalarmult(p, q, s);
}

function crypto_sign_keypair(pk, sk, hasher) {
	var d = new Uint8Array(64);
	var p = [gf(), gf(), gf(), gf()];
	var i;

	crypto_hash(d, sk, 32, hasher);
	d[0] &= 248;
	d[31] &= 127;
	d[31] |= 64;

	scalarbase(p, d);
	pack(pk, p);

	for (i = 0; i < 32; i++) sk[i+32] = pk[i];
	return 0;
}

var L = new Float64Array([0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10]);

function modL(r, x) {
	var carry, i, j, k;
	for (i = 63; i >= 32; --i) {
		carry = 0;
		for (j = i - 32, k = i - 12; j < k; ++j) {
			x[j] += carry - 16 * x[i] * L[j - (i - 32)];
			carry = Math.floor((x[j] + 128) / 256);
			x[j] -= carry * 256;
		}
		x[j] += carry;
		x[i] = 0;
	}
	carry = 0;
	for (j = 0; j < 32; j++) {
		x[j] += carry - (x[31] >> 4) * L[j];
		carry = x[j] >> 8;
		x[j] &= 255;
	}
	for (j = 0; j < 32; j++) x[j] -= carry * L[j];
	for (i = 0; i < 32; i++) {
		x[i+1] += x[i] >> 8;
		r[i] = x[i] & 255;
	}
}

function reduce(r) {
	var x = new Float64Array(64), i;
	for (i = 0; i < 64; i++) x[i] = r[i];
	for (i = 0; i < 64; i++) r[i] = 0;
	modL(r, x);
}

// Note: difference from C - smlen returned, not passed as argument.
function crypto_sign(sm, m, n, sk, hasher) {
	var d = new Uint8Array(64), h = new Uint8Array(64), r = new Uint8Array(64);
	var i, j, x = new Float64Array(64);
	var p = [gf(), gf(), gf(), gf()];

	crypto_hash(d, sk, 32, hasher);
	d[0] &= 248;
	d[31] &= 127;
	d[31] |= 64;

	var smlen = n + 64;
	for (i = 0; i < n; i++) sm[64 + i] = m[i];
	for (i = 0; i < 32; i++) sm[32 + i] = d[32 + i];

	crypto_hash(r, sm.subarray(32), n+32, hasher);
	reduce(r);
	scalarbase(p, r);
	pack(sm, p);

	for (i = 32; i < 64; i++) sm[i] = sk[i];
	crypto_hash(h, sm, n + 64, hasher);
	reduce(h);

	for (i = 0; i < 64; i++) x[i] = 0;
	for (i = 0; i < 32; i++) x[i] = r[i];
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j++) {
			x[i+j] += h[i] * d[j];
		}
	}

	modL(sm.subarray(32), x);
	return smlen;
}

function unpackneg(r, p) {
	var t = gf(), chk = gf(), num = gf(),
			den = gf(), den2 = gf(), den4 = gf(),
			den6 = gf();

	set25519(r[2], gf1);
	unpack25519(r[1], p);
	S(num, r[1]);
	M(den, num, D);
	Z(num, num, r[2]);
	A(den, r[2], den);

	S(den2, den);
	S(den4, den2);
	M(den6, den4, den2);
	M(t, den6, num);
	M(t, t, den);

	pow2523(t, t);
	M(t, t, num);
	M(t, t, den);
	M(t, t, den);
	M(r[0], t, den);

	S(chk, r[0]);
	M(chk, chk, den);
	if (neq25519(chk, num)) M(r[0], r[0], I);

	S(chk, r[0]);
	M(chk, chk, den);
	if (neq25519(chk, num)) return -1;

	if (par25519(r[0]) === (p[31]>>7)) Z(r[0], gf0, r[0]);

	M(r[3], r[0], r[1]);
	return 0;
}

function crypto_sign_open(m, sm, n, pk, hasher) {
	var i;
	var t = new Uint8Array(32), h = new Uint8Array(64);
	var p = [gf(), gf(), gf(), gf()],
			q = [gf(), gf(), gf(), gf()];

	if (n < 64) return -1;

	if (unpackneg(q, pk)) return -1;

	for (i = 0; i < n; i++) m[i] = sm[i];
	for (i = 0; i < 32; i++) m[i+32] = pk[i];
	crypto_hash(h, m, n, hasher);
	reduce(h);
	scalarmult(p, q, h);

	scalarbase(q, sm.subarray(32));
	add(p, q);
	pack(t, p);

	n -= 64; // eslint-disable-line no-param-reassign
	if (crypto_verify_32(sm, 0, t, 0)) {
		for (i = 0; i < n; i++) m[i] = 0;
		return -1;
	}

	for (i = 0; i < n; i++) m[i] = sm[i + 64];
	return n;
}

var crypto_sign_BYTES = 64,
		crypto_sign_PUBLICKEYBYTES = 32,
		crypto_sign_SECRETKEYBYTES = 64,
		crypto_sign_SEEDBYTES = 32;

const nacl = {};
nacl.lowlevel = {
	crypto_hash,

	gf,
	L,
	Z,
	modL,
	scalarmult,
	neq25519,
	par25519,
	inv25519,
	pack,
	unpackneg
};

/* High-level API */

function checkArrayTypes(...params) {
	for (var i = 0; i < params.length; i++) {
		if (!(params[i] instanceof Uint8Array))
			throw new TypeError('unexpected type, use Uint8Array');
	}
}

nacl.sign = function(msg, secretKey, hasher) {
	checkArrayTypes(msg, secretKey);
	if (secretKey.length !== crypto_sign_SECRETKEYBYTES)
		throw new Error('bad secret key size');
	var signedMsg = new Uint8Array(crypto_sign_BYTES+msg.length);
	crypto_sign(signedMsg, msg, msg.length, secretKey, hasher);
	return signedMsg;
};

nacl.sign.detached = function(msg, secretKey, hasher) {
	var signedMsg = nacl.sign(msg, secretKey, hasher);
	var sig = new Uint8Array(crypto_sign_BYTES);
	for (var i = 0; i < sig.length; i++) sig[i] = signedMsg[i];
	return sig;
};

nacl.sign.detached.verify = function(msg, sig, publicKey, hasher) {
	checkArrayTypes(msg, sig, publicKey);
	if (sig.length !== crypto_sign_BYTES)
		throw new Error('bad signature size');
	if (publicKey.length !== crypto_sign_PUBLICKEYBYTES)
		throw new Error('bad public key size');
	var sm = new Uint8Array(crypto_sign_BYTES + msg.length);
	var m = new Uint8Array(crypto_sign_BYTES + msg.length);
	var i;
	for (i = 0; i < crypto_sign_BYTES; i++) sm[i] = sig[i];
	for (i = 0; i < msg.length; i++) sm[i+crypto_sign_BYTES] = msg[i];
	return (crypto_sign_open(m, sm, sm.length, publicKey, hasher) >= 0);
};

nacl.sign.keyPair = {};
nacl.sign.keyPair.fromSeed = function(seed, hasher) {
	checkArrayTypes(seed);
	if (seed.length !== crypto_sign_SEEDBYTES)
		throw new Error('bad seed size');
	var pk = new Uint8Array(crypto_sign_PUBLICKEYBYTES);
	var sk = new Uint8Array(crypto_sign_SECRETKEYBYTES);
	for (var i = 0; i < 32; i++) sk[i] = seed[i];
	crypto_sign_keypair(pk, sk, hasher);
	return {publicKey: pk, secretKey: sk};
};

export default nacl;
