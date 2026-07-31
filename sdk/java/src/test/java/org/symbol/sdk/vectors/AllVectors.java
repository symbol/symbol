package org.symbol.sdk.vectors;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;
import java.util.function.Supplier;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.ByteArray;
import org.symbol.sdk.CipherTypes;
import org.symbol.sdk.CryptoTypes.PrivateKey;
import org.symbol.sdk.CryptoTypes.PublicKey;
import org.symbol.sdk.CryptoTypes.SharedKey256;
import org.symbol.sdk.CryptoTypes.Signature;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.Network;
import org.symbol.sdk.facade.BlockchainFacade;
import org.symbol.sdk.facade.FacadeFactory;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.VotingKeysGenerator;
import org.symbol.sdk.utils.Converter;

/**
 * Cross-language crypto vectors harness: runs the vectors under {@code tests/vectors/{nem,symbol}/crypto} through Java SDK primitives,
 * prints a per-suite summary, and exits non-zero on failure. Invoked via the {@code vectors} Gradle task.
 */
public final class AllVectors {

	private static final com.fasterxml.jackson.databind.ObjectMapper JSON_MAPPER = com.fasterxml.jackson.databind.json.JsonMapper.builder()
			.enable(com.fasterxml.jackson.databind.DeserializationFeature.USE_BIG_INTEGER_FOR_INTS).build();

	/** Parses a vectors JSON file into a plain value tree; integers parse as wide {@code Number}s so u64 values survive unclipped. */
	private static Object parseJsonFile(final java.nio.file.Path file) {
		try {
			return JSON_MAPPER.readValue(java.nio.file.Files.readString(file, java.nio.charset.StandardCharsets.UTF_8), Object.class);
		} catch (final java.io.IOException ex) {
			throw new java.io.UncheckedIOException(ex);
		}
	}
	private AllVectors() {
	}

	// region tester framework

	private abstract static class VectorsTestSuite {
		final int identifier;
		final String description;
		private final String filenameSuffix;

		VectorsTestSuite(final int identifier, final String filenameSuffix, final String description) {
			this.identifier = identifier;
			this.filenameSuffix = filenameSuffix;
			this.description = description;
		}

		String filename() {
			return identifier + "." + filenameSuffix;
		}

		abstract List<Pair> process(Map<String, Object> vector);
	}

	/** Expected/actual pair recorded by a tester. Equality compares deeply by content. */
	private record Pair(Object expected, Object actual) {
	}

	private record Arguments(String vectorsPath, String blockchain, Set<Integer> tests) {
	}

	// endregion

	// region hash tester

	// (beyond the JS runner, which registers no tester for the 0.* hash files)
	private static final class HashTester extends VectorsTestSuite {
		private final Function<byte[], byte[]> hasher;

		HashTester(final String filenameSuffix, final String description, final Function<byte[], byte[]> hasher) {
			super(0, filenameSuffix, description);
			this.hasher = hasher;
		}

		@Override
		List<Pair> process(final Map<String, Object> vector) {
			final byte[] expected = Converter.hexToUint8((String) vector.get("hash"));
			final byte[] actual = hasher.apply(Converter.hexToUint8((String) vector.get("data")));
			return List.of(new Pair(expected, actual));
		}
	}

	// endregion

	// region key/address testers

	private static final class KeyConversionTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		KeyConversionTester(final BlockchainFacade<?, ?, ?> facade) {
			super(1, "test-keys", "key conversion");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PrivateKey privateKey = new PrivateKey((String) v.get("privateKey"));
			final PublicKey expected = new PublicKey((String) v.get("publicKey"));
			final PublicKey actual = facade.createKeyPair(privateKey).getPublicKey();
			return List.of(new Pair(expected, actual));
		}
	}

	private static final class AddressConversionTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;
		private final BlockchainFacade<?, ?, ?> mainnetFacade;

		AddressConversionTester(final BlockchainFacade<?, ?, ?> facade, final BlockchainFacade<?, ?, ?> mainnetFacade) {
			super(1, "test-address", "address conversion");
			this.facade = facade;
			this.mainnetFacade = mainnetFacade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PublicKey publicKey = new PublicKey((String) v.get("publicKey"));
			final ByteArray expectedMain = facade.createAddress((String) v.get("address_Public"));
			final ByteArray expectedTest = facade.createAddress((String) v.get("address_PublicTest"));
			final Network<?, ?> mainnet = mainnetFacade.network();
			final Network<?, ?> testnet = facade.network();
			return List.of(new Pair(expectedMain, mainnet.publicKeyToAddress(publicKey)),
					new Pair(expectedTest, testnet.publicKeyToAddress(publicKey)));
		}
	}

	// endregion

	// region sign/verify

	private static final class SignTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		SignTester(final BlockchainFacade<?, ?, ?> facade) {
			super(2, "test-sign", "sign");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PrivateKey privateKey = new PrivateKey((String) v.get("privateKey"));
			final byte[] message = Converter.hexToUint8((String) v.get("data"));
			final Signature expected = new Signature((String) v.get("signature"));
			final Signature actual = facade.createKeyPair(privateKey).sign(message);
			return List.of(new Pair(expected, actual));
		}
	}

	private static final class VerifyTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		VerifyTester(final BlockchainFacade<?, ?, ?> facade) {
			super(2, "test-sign", "verify");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PublicKey publicKey = new PublicKey((String) v.get("publicKey"));
			final byte[] message = Converter.hexToUint8((String) v.get("data"));
			final Signature signature = new Signature((String) v.get("signature"));
			final boolean ok = facade.createVerifier(publicKey).verify(message, signature);
			return List.of(new Pair(Boolean.TRUE, ok));
		}
	}

	// endregion

	// region derive / cipher (modern + deprecated)

	private static final class DeriveTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		DeriveTester(final BlockchainFacade<?, ?, ?> facade) {
			super(3, "test-derive-hkdf", "derive");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PublicKey other = new PublicKey((String) v.get("otherPublicKey"));
			final PrivateKey privateKey = new PrivateKey((String) v.get("privateKey"));
			final SharedKey256 actual = deriveSharedKey(facade, privateKey, other);
			return List.of(new Pair(new SharedKey256((String) v.get("sharedKey")), actual));
		}
	}

	private static final class DeriveDeprecatedTester extends VectorsTestSuite {
		DeriveDeprecatedTester() {
			super(3, "test-derive-deprecated", "derive-deprecated");
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PublicKey other = new PublicKey((String) v.get("otherPublicKey"));
			final org.symbol.sdk.nem.KeyPair kp = new org.symbol.sdk.nem.KeyPair(new PrivateKey((String) v.get("privateKey")));
			final byte[] salt = Converter.hexToUint8((String) v.get("salt"));
			final SharedKey256 actual = org.symbol.sdk.nem.SharedKey.deriveSharedKeyDeprecated(kp, other, salt);
			return List.of(new Pair(new SharedKey256((String) v.get("sharedKey")), actual));
		}
	}

	private static final class CipherTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		CipherTester(final BlockchainFacade<?, ?, ?> facade) {
			super(4, "test-cipher", "cipher");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PublicKey other = new PublicKey((String) v.get("otherPublicKey"));
			final PrivateKey privateKey = new PrivateKey((String) v.get("privateKey"));
			final SharedKey256 sharedKey = deriveSharedKey(facade, privateKey, other);

			final byte[] iv = Converter.hexToUint8((String) v.get("iv"));
			final byte[] tag = Converter.hexToUint8((String) v.get("tag"));
			final byte[] cipherText = Converter.hexToUint8((String) v.get("cipherText"));
			final byte[] clearText = Converter.hexToUint8((String) v.get("clearText"));

			final CipherTypes.AesGcmCipher cipher = new CipherTypes.AesGcmCipher(sharedKey);
			final byte[] resultCipherText = cipher.encrypt(clearText, iv);

			final byte[] combined = new byte[cipherText.length + tag.length];
			System.arraycopy(cipherText, 0, combined, 0, cipherText.length);
			System.arraycopy(tag, 0, combined, cipherText.length, tag.length);

			final byte[] resultClearText = cipher.decrypt(combined, iv);
			return List.of(new Pair(combined, resultCipherText), new Pair(clearText, resultClearText));
		}
	}

	private static final class CipherDeprecatedTester extends VectorsTestSuite {
		CipherDeprecatedTester() {
			super(4, "test-cipher-deprecated", "cipher-deprecated");
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final PublicKey other = new PublicKey((String) v.get("otherPublicKey"));
			final org.symbol.sdk.nem.KeyPair kp = new org.symbol.sdk.nem.KeyPair(new PrivateKey((String) v.get("privateKey")));
			final byte[] salt = Converter.hexToUint8((String) v.get("salt"));
			final SharedKey256 sharedKey = org.symbol.sdk.nem.SharedKey.deriveSharedKeyDeprecated(kp, other, salt);

			final byte[] iv = Converter.hexToUint8((String) v.get("iv"));
			final byte[] cipherText = Converter.hexToUint8((String) v.get("cipherText"));
			final byte[] clearText = Converter.hexToUint8((String) v.get("clearText"));

			final CipherTypes.AesCbcCipher cipher = new CipherTypes.AesCbcCipher(sharedKey);
			final byte[] resultCipherText = cipher.encrypt(clearText, iv);
			final byte[] resultClearText = cipher.decrypt(cipherText, iv);
			return List.of(new Pair(cipherText, resultCipherText), new Pair(clearText, resultClearText));
		}
	}

	// endregion

	// region mosaic id, BIP32/BIP39, voting keys

	private static final class MosaicIdDerivationTester extends VectorsTestSuite {
		private static final String[] NETWORK_TAGS = {
				"Public", "PublicTest", "Private", "PrivateTest"
		};

		MosaicIdDerivationTester() {
			super(5, "test-mosaic-id", "mosaic id derivation");
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final long nonce = Converter.toLong((Number) v.get("mosaicNonce"));
			final List<Pair> pairs = new ArrayList<>();
			for (String tag : NETWORK_TAGS) {
				final org.symbol.sdk.symbol.Address address = new org.symbol.sdk.symbol.Address((String) v.get("address_" + tag));
				final long expected = Long.parseUnsignedLong((String) v.get("mosaicId_" + tag), 16);
				final long actual = IdGenerator.generateMosaicId(address, nonce);
				pairs.add(new Pair(expected, actual));
			}
			return pairs;
		}
	}

	private static final class Bip32DerivationTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		Bip32DerivationTester(final BlockchainFacade<?, ?, ?> facade) {
			super(6, "test-hd-derivation", "BIP32 derivation");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final byte[] seed = Converter.hexToUint8((String) v.get("seed"));
			final PublicKey expectedRoot = new PublicKey((String) v.get("rootPublicKey"));

			final Bip32.Bip32Node root = new Bip32(facade.bip32CurveName()).fromSeed(seed);
			final PublicKey actualRoot = facade.createKeyPair(root.privateKey).getPublicKey();

			final List<?> children = (List<?>) v.get("childAccounts");

			final List<PublicKey> expectedChildren = new ArrayList<>();
			final List<PublicKey> actualChildren = new ArrayList<>();
			for (Object childObject : children) {
				final Map<?, ?> child = (Map<?, ?>) childObject;
				expectedChildren.add(new PublicKey((String) child.get("publicKey")));
				final List<?> rawPath = (List<?>) child.get("path");
				final int[] path = new int[rawPath.size()];
				for (int i = 0; i < path.length; ++i)
					path[i] = ((Number) rawPath.get(i)).intValue();
				final Bip32.Bip32Node childNode = root.derivePath(path);
				actualChildren.add(facade.bip32NodeToKeyPair(childNode).getPublicKey());
			}

			return List.of(new Pair(expectedRoot, actualRoot), new Pair(expectedChildren, actualChildren));
		}
	}

	private static final class Bip39DerivationTester extends VectorsTestSuite {
		private final BlockchainFacade<?, ?, ?> facade;

		Bip39DerivationTester(final BlockchainFacade<?, ?, ?> facade) {
			super(6, "test-hd-derivation", "BIP39 derivation");
			this.facade = facade;
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			if (!v.containsKey("mnemonic"))
				return List.of();

			final String mnemonic = (String) v.get("mnemonic");
			final String passphrase = (String) v.get("passphrase");
			final PublicKey expectedRoot = new PublicKey((String) v.get("rootPublicKey"));
			final Bip32.Bip32Node root = new Bip32(facade.bip32CurveName()).fromMnemonic(mnemonic, passphrase);
			final PublicKey actualRoot = facade.createKeyPair(root.privateKey).getPublicKey();
			return List.of(new Pair(expectedRoot, actualRoot));
		}
	}

	private static final class VotingKeysGenerationTester extends VectorsTestSuite {
		VotingKeysGenerationTester() {
			super(7, "test-voting-keys-generation", "voting keys generation");
		}

		@Override
		List<Pair> process(final Map<String, Object> v) {
			final Supplier<PrivateKey> generator = switch ((String) v.get("name")) {
				case "test_vector_1" -> new FibPrivateKeyGenerator(false);
				case "test_vector_2" -> new FibPrivateKeyGenerator(true);
				case "test_vector_3" -> new SeededPrivateKeyGenerator(
						List.of(new PrivateKey("12F98B7CB64A6D840931A2B624FB1EACAFA2C25C3EF0018CD67E8D470A248B2F"),
								new PrivateKey("B5593870940F28DAEE262B26367B69143AD85E43048D23E624F4ED8008C0427F"),
								new PrivateKey("6CFC879ABCCA78F5A4C9739852C7C643AEC3990E93BF4C6F685EB58224B16A59")));
				default -> throw new IllegalArgumentException("unknown voting vector name " + v.get("name"));
			};

			final PrivateKey rootPrivateKey = new PrivateKey((String) v.get("rootPrivateKey"));
			final VotingKeysGenerator vkg = new VotingKeysGenerator(new org.symbol.sdk.symbol.KeyPair(rootPrivateKey), generator);

			final long startEpoch = Converter.toLong((Number) v.get("startEpoch"));
			final long endEpoch = Converter.toLong((Number) v.get("endEpoch"));
			final byte[] actual = vkg.generate(startEpoch, endEpoch);
			final byte[] expected = Converter.hexToUint8((String) v.get("expectedFileHex"));
			return List.of(new Pair(expected, actual));
		}
	}

	private static final class SeededPrivateKeyGenerator implements Supplier<PrivateKey> {
		private final List<PrivateKey> values;
		private int next;

		SeededPrivateKeyGenerator(final List<PrivateKey> values) {
			this.values = values;
		}

		@Override
		public PrivateKey get() {
			return values.get(next++);
		}
	}

	private static final class FibPrivateKeyGenerator implements Supplier<PrivateKey> {
		private final boolean fill;
		private int value1 = 1;
		private int value2 = 2;

		FibPrivateKeyGenerator(final boolean fill) {
			this.fill = fill;
		}

		@Override
		public PrivateKey get() {
			final int next = value1 + value2;
			value1 = value2;
			value2 = next;
			final int seedValue = Math.floorMod(next, 256);
			final byte[] buffer = new byte[PrivateKey.SIZE];
			if (fill) {
				for (int i = 0; i < PrivateKey.SIZE; ++i)
					buffer[i] = (byte) ((seedValue + i) % 256);
			} else {
				buffer[PrivateKey.SIZE - 1] = (byte) seedValue;
			}

			return new PrivateKey(buffer);
		}
	}

	// endregion

	// region orchestration

	// capture the facade's key-pair type so createKeyPair's result feeds deriveSharedKey (both on the facade contract, JS parity)
	private static <K extends KeyPair> SharedKey256 deriveSharedKey(final BlockchainFacade<?, ?, K> facade, final PrivateKey privateKey,
			final PublicKey otherPublicKey) {
		return facade.deriveSharedKey(facade.createKeyPair(privateKey), otherPublicKey);
	}

	private static List<VectorsTestSuite> loadTestSuites(final String blockchain) {
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create(blockchain, "testnet");
		final BlockchainFacade<?, ?, ?> mainnetFacade = FacadeFactory.create(blockchain, "mainnet");
		final List<VectorsTestSuite> suites = new ArrayList<>(List.of(new KeyConversionTester(facade),
				new AddressConversionTester(facade, mainnetFacade), new SignTester(facade), new VerifyTester(facade),
				new DeriveTester(facade), new CipherTester(facade), new Bip32DerivationTester(facade), new Bip39DerivationTester(facade)));
		if ("symbol".equals(blockchain)) {
			suites.add(new HashTester("test-sha3-256", "sha3-256", org.symbol.sdk.utils.Transforms::sha3_256));
			suites.add(new HashTester("test-keccak-256", "keccak-256", org.symbol.sdk.utils.Transforms::keccak_256));
			suites.add(new MosaicIdDerivationTester());
			suites.add(new VotingKeysGenerationTester());
		} else {
			suites.add(new DeriveDeprecatedTester());
			suites.add(new CipherDeprecatedTester());
		}

		return suites;
	}

	private static String requireValue(final String[] args, final int index, final String option) {
		if (index + 1 >= args.length)
			throw new IllegalArgumentException("missing value for " + option);

		return args[index + 1];
	}

	private static Arguments parseArgs(final String[] args) {
		String vectorsPath = null;
		String blockchain = "symbol";
		Set<Integer> tests = null;

		for (int i = 0; i < args.length; ++i) {
			switch (args[i]) {
				case "--vectors":
					vectorsPath = requireValue(args, i, "--vectors");
					++i;
					break;
				case "--blockchain":
					blockchain = requireValue(args, i, "--blockchain");
					++i;
					break;
				case "--tests":
					tests = new HashSet<>();
					while (i + 1 < args.length && !args[i + 1].startsWith("--"))
						tests.add(Integer.parseInt(args[++i]));
					break;
				default :
					throw new IllegalArgumentException("unknown argument " + args[i]);
			}
		}

		if (vectorsPath == null)
			throw new IllegalArgumentException("--vectors is required");

		if (!"nem".equals(blockchain) && !"symbol".equals(blockchain))
			throw new IllegalArgumentException("--blockchain must be nem or symbol");

		return new Arguments(vectorsPath, blockchain, tests);
	}

	private static List<Map<String, Object>> collectCases(final Object parsed) {
		if (parsed instanceof List<?> top)
			return CatbufferVectorsHelper.toObjectMaps(top);

		if (parsed instanceof Map<?, ?> top) {
			final List<Map<String, Object>> cases = new ArrayList<>();
			for (final Object groupValue : top.values()) {
				if (!(groupValue instanceof List<?> group))
					throw new IllegalArgumentException("unexpected grouped vectors shape: expected list group");

				cases.addAll(CatbufferVectorsHelper.toObjectMaps(group));
			}

			return cases;
		}

		throw new IllegalArgumentException("unexpected vectors root shape: expected list or object");
	}

	private static boolean areDifferent(final Object lhs, final Object rhs) {
		if (lhs == null || rhs == null)
			return lhs != rhs;

		if (lhs instanceof byte[] lb && rhs instanceof byte[] rb)
			return !Arrays.equals(lb, rb);

		if (lhs instanceof ByteArray lba && rhs instanceof ByteArray rba)
			return !Arrays.equals(lba.bytes(), rba.bytes());

		if (lhs instanceof List<?> ll && rhs instanceof List<?> rl) {
			if (ll.size() != rl.size())
				return true;

			for (int i = 0; i < ll.size(); ++i) {
				if (areDifferent(ll.get(i), rl.get(i)))
					return true;
			}
			return false;
		}

		return !lhs.equals(rhs);
	}

	private static int runSuite(final VectorsTestSuite suite, final String vectorsPath) {
		final Path file = Paths.get(vectorsPath, suite.filename() + ".json");
		final Object parsed = parseJsonFile(file);
		final long startNanos = System.nanoTime();

		int testCaseNumber = 0;
		int numFailed = 0;
		final List<Map<String, Object>> cases = collectCases(parsed);

		for (Map<String, Object> testCase : cases) {
			final List<Pair> pairs = suite.process(testCase);
			if (pairs.isEmpty())
				// a tester returns no pairs for a case it deliberately skips (e.g. BIP39 vectors
				// without mnemonic)
				continue;

			if (pairs.stream().anyMatch(pair -> areDifferent(pair.expected(), pair.actual())))
				++numFailed;

			++testCaseNumber;
		}

		final double elapsed = (System.nanoTime() - startNanos) / 1_000_000_000.0;
		if (0 == testCaseNumber) {
			System.out.printf(Locale.ROOT, "[%.4fs] %s test: no cases ran (empty or fully-skipped vector file)%n", elapsed,
					suite.description);
			return 1;
		}

		return reportSuiteResult(suite.description, elapsed, numFailed, testCaseNumber) ? 1 : 0;
	}

	private static boolean reportSuiteResult(final String description, final double elapsed, final int numFailed,
			final int testCaseNumber) {
		final String prefix = String.format(Locale.ROOT, "[%.4fs] %s test:", elapsed, description);
		if (numFailed > 0) {
			System.out.printf(Locale.ROOT, "%s %d failures out of %d%n", prefix, numFailed, testCaseNumber);
			return true;
		}

		System.out.printf(Locale.ROOT, "%s successes %d%n", prefix, testCaseNumber);
		return false;
	}

	/**
	 * Entry point invoked by the {@code vectors} Gradle task.
	 *
	 * @param args Command-line arguments: {@code --vectors PATH --blockchain {nem|symbol} [--tests N...]}.
	 */
	public static void main(final String[] args) {
		final Arguments parsedArgs = parseArgs(args);
		System.out.printf(Locale.ROOT, "running tests for %s blockchain with vectors from %s%n", parsedArgs.blockchain(),
				parsedArgs.vectorsPath());

		int numFailedSuites = 0;
		int numSuitesRun = 0;
		for (VectorsTestSuite suite : loadTestSuites(parsedArgs.blockchain())) {
			if (parsedArgs.tests() != null && !parsedArgs.tests().contains(suite.identifier)) {
				System.out.println("[ SKIPPED ] " + suite.description + " test");
				continue;
			}

			++numSuitesRun;
			numFailedSuites += runSuite(suite, parsedArgs.vectorsPath());
		}

		if (0 == numSuitesRun) {
			System.out.println("no test suites ran — check the --tests filter " + parsedArgs.tests());
			System.exit(1);
		}

		if (numFailedSuites > 0)
			System.exit(1);
	}

	// endregion
}
