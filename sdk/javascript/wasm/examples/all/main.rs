extern crate clap;
use clap::Parser;

use std::fs;

extern crate serde;
use serde::{Deserialize, Serialize};
use serde::de::DeserializeOwned;

extern crate serde_hex;
use serde_hex::{SerHex, Strict};

extern crate serde_json;

use symbol_crypto_wasm::{
	HashMode, crypto_sign_keypair_unboxed, crypto_private_sign_unboxed, crypto_private_verify_unboxed,
	SECRET_KEY_LENGTH, PUBLIC_KEY_LENGTH, SIGNATURE_LENGTH
};

// region TestVectorDescriptor

struct TestVectorDescriptor {
	identifier: i64,
	description: String,
	_filename: String
}

impl TestVectorDescriptor {
	fn create(identifier: i64, filename: &str, description: &str) -> TestVectorDescriptor {
		return TestVectorDescriptor{ identifier: identifier, description: String::from(description), _filename: String::from(filename) };
	}

	fn filename(&self) -> String {
		return format!("{}.{}.json", self.identifier, self._filename);
	}
}

// endregion

// region prepare_secret_key / get_hash_mode

fn prepare_secret_key(blockchain: &String, private_key: &[u8; SECRET_KEY_LENGTH]) -> [u8; SECRET_KEY_LENGTH] {
	if "nem" != blockchain {
		return *private_key;
	}

	let mut secret_key: [u8; SECRET_KEY_LENGTH] = [0u8; SECRET_KEY_LENGTH];
	for i in 0..SECRET_KEY_LENGTH {
		secret_key[i] = private_key[SECRET_KEY_LENGTH - 1 - i];
	}

	return secret_key;
}

fn get_hash_mode(blockchain: &String) -> HashMode {
	if "nem" == blockchain { HashMode::Keccak } else { HashMode::Sha2_512 }
}

// endregion

// region key_conversion_tester

#[derive(Serialize, Deserialize)]
#[derive(Debug)]
#[serde(rename_all = "camelCase")]
struct KeyConversionTestVector {
	#[serde(with = "SerHex::<Strict>")]
	private_key: [u8; SECRET_KEY_LENGTH],

	#[serde(with = "SerHex::<Strict>")]
	public_key: [u8; PUBLIC_KEY_LENGTH]
}

fn key_conversion_tester(blockchain: &String, test_vector: &KeyConversionTestVector) -> bool {
	// Act:
	let public_key = crypto_sign_keypair_unboxed(
		get_hash_mode(blockchain),
		&mut prepare_secret_key(&blockchain, &test_vector.private_key));

	// Assert:
	return test_vector.public_key == public_key
}

// endregion

// region sign_tester

#[derive(Serialize, Deserialize)]
#[derive(Debug)]
#[serde(rename_all = "camelCase")]
struct SignTestVector {
	#[serde(with = "SerHex::<Strict>")]
	private_key: [u8; SECRET_KEY_LENGTH],

	#[serde(with = "SerHex::<Strict>")]
	public_key: [u8; PUBLIC_KEY_LENGTH],

	#[serde(with = "hex::serde")]
	data: Vec<u8>,

	length: u32,

	#[serde(with = "SerHex::<Strict>")]
	signature: [u8; SIGNATURE_LENGTH],
}

fn sign_tester(blockchain: &String, test_vector: &SignTestVector) -> bool {
	// Act:
	let signature = crypto_private_sign_unboxed(
		get_hash_mode(blockchain),
		&mut prepare_secret_key(&blockchain, &test_vector.private_key),
		&test_vector.data);

	// Assert:
	return test_vector.signature == signature
}

// endregion

// region verify_tester

#[derive(Serialize, Deserialize)]
#[derive(Debug)]
#[serde(rename_all = "camelCase")]
struct VerifyTestVector {
	#[serde(with = "SerHex::<Strict>")]
	public_key: [u8; PUBLIC_KEY_LENGTH],

	#[serde(with = "hex::serde")]
	data: Vec<u8>,

	length: u32,

	#[serde(with = "SerHex::<Strict>")]
	signature: [u8; SIGNATURE_LENGTH],
}

fn verify_tester(blockchain: &String, test_vector: &VerifyTestVector) -> bool {
	// Act:
	let is_verified = crypto_private_verify_unboxed(
		get_hash_mode(blockchain),
		&test_vector.public_key,
		&test_vector.data,
		&test_vector.signature);

	// Assert:
	return is_verified
}

// endregion

// region Cli

/// Test vectors command line options.
#[derive(Parser)]
struct Cli {
	/// Blockchain to run vectors against.
	#[clap(short, long, value_parser(["nem", "symbol"]))]
	blockchain: String,

	/// Path to test-vectors directory.
	#[clap(short, long, value_parser)]
	vectors: std::path::PathBuf,

	/// Identifiers of test suites to run.
	#[clap(short, long, num_args(0..), value_parser(1..=2))]
	tests: Vec<i64>,
}

// endregion

// region TestRunner

fn load_test_cases<T: DeserializeOwned>(vectors_directory: &std::path::PathBuf, descriptor: &TestVectorDescriptor) -> Vec<T> {
	let mut vectors_filepath: std::path::PathBuf = vectors_directory.clone();
	vectors_filepath.push(descriptor.filename());

	let contents = fs::read_to_string(vectors_filepath)
		.expect("unable to read file");
	return serde_json::from_str::<Vec<T>>(&contents).unwrap();
}

fn run_test_cases<T>(
		blockchain: &String,
		descriptor: &TestVectorDescriptor,
		test_cases: &Vec<T>,
		tester: &dyn Fn(&String, &T) -> bool) -> bool {
	use std::time::Instant;
	let timer = Instant::now();

	let mut test_case_number = 0;
	let mut num_failed = 0;

	for test_case in test_cases {
		if !tester(&blockchain, &test_case) {
			num_failed += 1;
		}

		test_case_number += 1;
	}

	let elapsed_time = timer.elapsed();

	let test_message_prefix = format!("[{:.4}s] {} test:", elapsed_time.as_secs_f64(), descriptor.description);
	if 0 != num_failed {
		println!("{} {} failures out of {}", test_message_prefix, num_failed, test_case_number);
		return false;
	}

	println!("{} successes {}", test_message_prefix, test_case_number);
	return true;
}

fn load_and_run_test_cases<T: DeserializeOwned>(
		vectors_directory: &std::path::PathBuf,
		blockchain: &String,
		descriptor: &TestVectorDescriptor,
		tester: &dyn Fn(&String, &T) -> bool) -> bool {
	let test_cases = load_test_cases(&vectors_directory, &descriptor);
	return run_test_cases(&blockchain, &descriptor, &test_cases, tester);
}

fn main() {
	let args = Cli::parse();

	println!("running tests for {} blockchain with vectors from {}", args.blockchain, args.vectors.display());
	if !args.tests.is_empty() {
		println!("selected suites: {:?}", args.tests);
	}

	type TestRunner = fn(&Cli, &TestVectorDescriptor) -> bool;
	let key_conversion_test_runner: TestRunner = |args: &Cli, descriptor: &TestVectorDescriptor| -> bool {
		return load_and_run_test_cases(&args.vectors, &args.blockchain, descriptor, &key_conversion_tester)
	};
	let sign_test_runner: TestRunner = |args: &Cli, descriptor: &TestVectorDescriptor| -> bool {
		return load_and_run_test_cases(&args.vectors, &args.blockchain, descriptor, &sign_tester)
	};
	let verify_test_runner: TestRunner = |args: &Cli, descriptor: &TestVectorDescriptor| -> bool {
		return load_and_run_test_cases(&args.vectors, &args.blockchain, descriptor, &verify_tester)
	};

	let test_suites = [
		(TestVectorDescriptor::create(1, "test-keys", "key conversion"), &key_conversion_test_runner),
		(TestVectorDescriptor::create(2, "test-sign", "sign"), &sign_test_runner),
		(TestVectorDescriptor::create(2, "test-sign", "verify"), &verify_test_runner)
	];

	let mut num_failed_suites = 0;
	for test_suite in test_suites {
		if !args.tests.is_empty() && !args.tests.contains(&test_suite.0.identifier) {
			println!("[ SKIPPED ] {} test", test_suite.0.description);
			continue;
		}

		if !test_suite.1(&args, &test_suite.0) {
			num_failed_suites += 1;
		}
	}

	if 0 != num_failed_suites {
		std::process::exit(1);
	}
}

// endregion
