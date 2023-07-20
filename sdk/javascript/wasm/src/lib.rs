// -*- mode: rust; -*-

//! ed25519 rust implementation and wasm bindings

#![no_std]
#![warn(future_incompatible)]
#![deny(missing_docs)]

#![cfg_attr(not(test), forbid(unsafe_code))]

extern crate curve25519_dalek;
use curve25519_dalek::scalar::Scalar;
use curve25519_dalek::edwards::{CompressedEdwardsY, EdwardsPoint};

extern crate serde;

use sha2::{Digest as Sha2Digest, Sha512};
use sha3::{Keccak512};

use js_sys::Uint8Array;
use wasm_bindgen::prelude::*;
use zeroize::Zeroize;

mod utils;
use crate::utils::set_panic_hook;

/// Length of an ed25519 secret key, in bytes.
pub const SECRET_KEY_LENGTH: usize = 32;

/// Length of an ed25519 public key, in bytes.
pub const PUBLIC_KEY_LENGTH: usize = 32;

/// Length of a private key hash, in bytes.
pub const HASH_LENGTH: usize = 64;

/// Length of a signature, in bytes.
pub const SIGNATURE_LENGTH: usize = 64;

/// Supported hash modes.
#[derive(Copy)]
#[derive(Clone)]
#[derive(PartialEq)]
#[wasm_bindgen]
pub enum HashMode {
	/// Keccak hash.
	Keccak,

	/// SHA2 hash.
	Sha2_512
}

// region Hasher512

struct Hasher512 {
	hash_mode: HashMode,
	_keccak: Keccak512,
	_sha2: Sha512
}

impl Hasher512 {
	fn new(hash_mode: HashMode) -> Hasher512 {
		return Hasher512{
			hash_mode: hash_mode,
			_keccak: Keccak512::new(),
			_sha2: Sha512::new()
		}
	}

	fn update<T: AsRef<[u8]> + ?Sized>(&mut self, input: &T) {
		if HashMode::Keccak == self.hash_mode {
			self._keccak.update(&input);
		} else {
			self._sha2.update(&input);
		}
	}

	fn finalize_into(self, out: &mut [u8; HASH_LENGTH]) {
		if HashMode::Keccak == self.hash_mode {
			out.copy_from_slice(&self._keccak.finalize().as_slice());
		} else {
			out.copy_from_slice(&self._sha2.finalize().as_slice());
		}
	}
}

// endregion

// region helpers

fn hash_private_key(hash_mode: HashMode, sk: &[u8; SECRET_KEY_LENGTH]) -> [u8; HASH_LENGTH] {
	let mut hasher = Hasher512::new(hash_mode);
	hasher.update(&sk);

	let mut hash: [u8; HASH_LENGTH] = [0u8; HASH_LENGTH];
	hasher.finalize_into(&mut hash);
	hash
}

fn copy_and_clamp_private_hash(private_hash: &[u8; HASH_LENGTH]) -> Scalar {
	let mut bits: [u8; SECRET_KEY_LENGTH] = [0; SECRET_KEY_LENGTH];
	bits.copy_from_slice(&private_hash[..SECRET_KEY_LENGTH]);

	bits[0] &= 248;
	bits[31] &= 127;
	bits[31] |= 64;

	Scalar::from_bits(*&mut bits)
}

fn hash_to_scalar(builder: Hasher512) -> Scalar {
	let mut hash = [0u8; HASH_LENGTH];
	builder.finalize_into(&mut hash);
	Scalar::from_bytes_mod_order_wide(&hash)
}

fn generate_nonce(hash_mode: HashMode, private_hash: &[u8; HASH_LENGTH], message: &[u8]) -> Scalar {
	// hash half of priv key hash
	let mut builder = Hasher512::new(hash_mode);
	builder.update(&private_hash[32..64]);
	builder.update(&message);

	hash_to_scalar(builder)
}

// endregion

// region implementations

/// Generates public key given secret key.
pub fn crypto_sign_keypair_unboxed(hash_mode: HashMode, sk: &[u8; SECRET_KEY_LENGTH]) -> [u8; PUBLIC_KEY_LENGTH] {
	let mut private_hash = hash_private_key(hash_mode, sk);
	let mut key = copy_and_clamp_private_hash(&private_hash);
	private_hash.zeroize();

	let point = &key * &curve25519_dalek::constants::ED25519_BASEPOINT_TABLE;
	key.zeroize();

	let compressed = point.compress();
	compressed.to_bytes()
}

/// Signs message.
pub fn crypto_private_sign_unboxed(hash_mode: HashMode, sk: &[u8; SECRET_KEY_LENGTH], message: &[u8]) -> [u8; SIGNATURE_LENGTH] {
	let mut private_hash = hash_private_key(hash_mode, sk);

	// R = rModQ * base point
	let r = generate_nonce(hash_mode, &private_hash, &message);
	#[allow(non_snake_case)]
	let R: CompressedEdwardsY = (&r * &curve25519_dalek::constants::ED25519_BASEPOINT_TABLE).compress();

	// h = H(encodedR || public || data)
	let public_key = crypto_sign_keypair_unboxed(hash_mode, &sk);

	let mut builder_h = Hasher512::new(hash_mode);
	builder_h.update(R.as_bytes());
	builder_h.update(&public_key);
	builder_h.update(&message);
	let h = hash_to_scalar(builder_h);

	// S = (r + h * a) mod group order
	let a = copy_and_clamp_private_hash(&private_hash);
	private_hash.zeroize();

	#[allow(non_snake_case)]
	let S = &(&h * &a) + &r;

	// serialize and concat R S
	let mut signature_bytes: [u8; SIGNATURE_LENGTH] = [0u8; SIGNATURE_LENGTH];
	signature_bytes[..32].copy_from_slice(&R.as_bytes()[..]);
	signature_bytes[32..].copy_from_slice(&S.as_bytes()[..]);

	signature_bytes
}

/// Verifies signature.
pub fn crypto_private_verify_unboxed(
		hash_mode: HashMode,
		pk: &[u8; PUBLIC_KEY_LENGTH],
		message: &[u8],
		signature: &[u8; SIGNATURE_LENGTH]) -> bool {
	// reject zero public key, which is known weak key
	if [0u8; PUBLIC_KEY_LENGTH] == *pk {
		return false;
	}

	// check s scalar before hashing
	let mut upper: [u8; 32] = [0u8; 32];
	upper.copy_from_slice(&signature[32..]);
	let s: Scalar = Scalar::from_bits(upper);
	if Scalar::zero() == s || !s.is_canonical() {
		return false;
	}

	// h = H(encodedR || public || data)
	let mut builder_h = Hasher512::new(hash_mode);
	builder_h.update(&signature[..32]);
	builder_h.update(&pk);
	builder_h.update(&message);
	let h = hash_to_scalar(builder_h);

	// unpack signature
	let mut lower: [u8; 32] = [0u8; 32];
	lower.copy_from_slice(&signature[..32]);
	#[allow(non_snake_case)]
	let R: EdwardsPoint = match CompressedEdwardsY(lower).decompress() {
		Some(EdwardsPoint) => EdwardsPoint,
		None => return false,
	};

	// unpack public key
	#[allow(non_snake_case)]
	let A = match CompressedEdwardsY(*pk).decompress() {
		Some(EdwardsPoint) => EdwardsPoint,
		None => return false,
	};

	// validate if public key is of small order or is outside of main subgroup
	// note: first check is actually cosmetic, as that should _not_ be the case after decompress()
	if A.is_small_order() || !A.is_torsion_free() {
		return false;
	}

	// check
	#[allow(non_snake_case)]
	let calculated_R = EdwardsPoint::vartime_double_scalar_mul_basepoint(&h, &-A, &s);
	return calculated_R == R;
}

// endregion

// region wasm bindings

/// Generates public key given secret key.
#[wasm_bindgen]
pub fn crypto_sign_keypair(hash_mode: HashMode, sk_boxed: &Uint8Array, pk_boxed: &Uint8Array) {
	set_panic_hook();

	let mut sk_bytes: [u8; SECRET_KEY_LENGTH] = [0; SECRET_KEY_LENGTH];
	sk_boxed.slice(0, SECRET_KEY_LENGTH as u32).copy_to(&mut sk_bytes);

	let pk_bytes = crypto_sign_keypair_unboxed(hash_mode, &sk_bytes);
	pk_boxed.copy_from(&pk_bytes);
}

/// Signs message.
#[wasm_bindgen]
pub fn crypto_private_sign(hash_mode: HashMode, sk_boxed: &Uint8Array, message_boxed: &Uint8Array, signature_boxed: &Uint8Array) {
	set_panic_hook();

	let mut sk_bytes: [u8; SECRET_KEY_LENGTH] = [0; SECRET_KEY_LENGTH];
	sk_boxed.slice(0, SECRET_KEY_LENGTH as u32).copy_to(&mut sk_bytes);

	let message = message_boxed.to_vec();
	let signature_bytes = crypto_private_sign_unboxed(hash_mode, &sk_bytes, message.as_slice());
	signature_boxed.copy_from(&signature_bytes);
}

/// Verifies signature.
#[wasm_bindgen]
pub fn crypto_private_verify(hash_mode: HashMode, pk_boxed: &Uint8Array, message_boxed: &Uint8Array, signature_boxed: &Uint8Array) -> bool {
	set_panic_hook();

	let mut pk_bytes: [u8; PUBLIC_KEY_LENGTH] = [0; PUBLIC_KEY_LENGTH];
	pk_boxed.slice(0, PUBLIC_KEY_LENGTH as u32).copy_to(&mut pk_bytes);

	let mut signature_bytes: [u8; SIGNATURE_LENGTH] = [0; SIGNATURE_LENGTH];
	signature_boxed.slice(0, SIGNATURE_LENGTH as u32).copy_to(&mut signature_bytes);

	let message = message_boxed.to_vec();
	crypto_private_verify_unboxed(hash_mode, &pk_bytes, message.as_slice(), &signature_bytes)
}

// endregion
