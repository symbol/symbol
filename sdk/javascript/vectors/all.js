import { Bip32 } from '../src/Bip32.js';
import { AesCbcCipher, AesGcmCipher } from '../src/Cipher.js';
import {
	PrivateKey, PublicKey, SharedKey256, Signature
} from '../src/CryptoTypes.js';
import { NetworkLocator } from '../src/Network.js';
import NemFacade from '../src/facade/NemFacade.js';
import SymbolFacade from '../src/facade/SymbolFacade.js';
import { Network as NemNetwork } from '../src/nem/Network.js';
import { deriveSharedKeyDeprecated } from '../src/nem/SharedKey.js'; // eslint-disable-line import/no-deprecated
import { Network as SymbolNetwork } from '../src/symbol/Network.js';
import VotingKeysGenerator from '../src/symbol/VotingKeysGenerator.js';
import { generateMosaicId } from '../src/symbol/idGenerator.js';
import { hexToUint8 } from '../src/utils/converter.js';
import yargs from 'yargs';
import fs from 'fs';
import path from 'path';

(() => {
	// region VectorsTestSuite, KeyConversionTester, AddressConversionTester

	class VectorsTestSuite {
		constructor(identifier, filename, description) {
			this.identifier = identifier;
			this._filename = filename;
			this.description = description;
		}

		get filename() {
			return `${this.identifier}.${this._filename}`;
		}
	}

	class KeyConversionTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(1, 'test-keys', 'key conversion');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const privateKey = new PrivateKey(testVector.privateKey);
			const expectedPublicKey = new PublicKey(testVector.publicKey);

			// Act:
			const actualPublicKey = new this.classLocator.Facade.KeyPair(privateKey).publicKey;

			// Assert:
			return [[expectedPublicKey, actualPublicKey]];
		}
	}

	class AddressConversionTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(1, 'test-address', 'address conversion');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const publicKey = new PublicKey(testVector.publicKey);
			const expectedAddressMainnet = new this.classLocator.Facade.Address(testVector.address_Public);
			const expectedAddressTestnet = new this.classLocator.Facade.Address(testVector.address_PublicTest);

			const mainnet = NetworkLocator.findByName(this.classLocator.Network.NETWORKS, 'mainnet');
			const testnet = NetworkLocator.findByName(this.classLocator.Network.NETWORKS, 'testnet');

			// Act:
			const actualAddressMainnet = mainnet.publicKeyToAddress(publicKey);
			const actualAddressTestnet = testnet.publicKeyToAddress(publicKey);

			// Assert:
			return [
				[expectedAddressMainnet, actualAddressMainnet],
				[expectedAddressTestnet, actualAddressTestnet]
			];
		}
	}

	// endregion

	// region SignTester, VerifyTester

	class SignTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(2, 'test-sign', 'sign');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const privateKey = new PrivateKey(testVector.privateKey);
			const message = hexToUint8(testVector.data);
			const expectedSignature = new Signature(testVector.signature);

			// Act:
			const actualSignature = new this.classLocator.Facade.KeyPair(privateKey).sign(message);

			// Assert:
			return [[expectedSignature, actualSignature]];
		}
	}

	class VerifyTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(2, 'test-sign', 'verify');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const publicKey = new PublicKey(testVector.publicKey);
			const message = hexToUint8(testVector.data);
			const signature = new Signature(testVector.signature);

			// Act:
			const isVerified = new this.classLocator.Facade.Verifier(publicKey).verify(message, signature);

			// Assert:
			return [[true, isVerified]];
		}
	}

	// endregion

	// region DeriveDeprecatedTester, DeriveTester, CipherDeprecatedTester, CipherTester

	class DeriveDeprecatedTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(3, 'test-derive-deprecated', 'derive-deprecated');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const otherPublicKey = new PublicKey(testVector.otherPublicKey);
			const keyPair = new this.classLocator.Facade.KeyPair(new PrivateKey(testVector.privateKey));
			const salt = hexToUint8(testVector.salt);

			// Act:
			const sharedKey = deriveSharedKeyDeprecated(keyPair, otherPublicKey, salt); // eslint-disable-line import/no-deprecated

			// Assert:
			return [[new SharedKey256(testVector.sharedKey), sharedKey]];
		}
	}

	class DeriveTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(3, 'test-derive-hkdf', 'derive');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const otherPublicKey = new PublicKey(testVector.otherPublicKey);
			const keyPair = new this.classLocator.Facade.KeyPair(new PrivateKey(testVector.privateKey));

			// Act:
			const sharedKey = this.classLocator.Facade.deriveSharedKey(keyPair, otherPublicKey);

			// Assert:
			return [[new SharedKey256(testVector.sharedKey), sharedKey]];
		}
	}

	class CipherDeprecatedTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(4, 'test-cipher-deprecated', 'cipher-deprecated');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const otherPublicKey = new PublicKey(testVector.otherPublicKey);
			const keyPair = new this.classLocator.Facade.KeyPair(new PrivateKey(testVector.privateKey));
			const salt = hexToUint8(testVector.salt);
			const sharedKey = deriveSharedKeyDeprecated(keyPair, otherPublicKey, salt); // eslint-disable-line import/no-deprecated

			const iv = hexToUint8(testVector.iv);
			const cipherText = hexToUint8(testVector.cipherText);
			const clearText = hexToUint8(testVector.clearText);

			// Act:
			const cipher = new AesCbcCipher(sharedKey);
			const resultCipherText = cipher.encrypt(clearText, iv);
			const resultClearText = cipher.decrypt(cipherText, iv);

			// Assert:
			return [
				[cipherText, resultCipherText],
				[clearText, resultClearText]
			];
		}
	}

	class CipherTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(4, 'test-cipher', 'cipher');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const otherPublicKey = new PublicKey(testVector.otherPublicKey);
			const keyPair = new this.classLocator.Facade.KeyPair(new PrivateKey(testVector.privateKey));
			const sharedKey = this.classLocator.Facade.deriveSharedKey(keyPair, otherPublicKey);

			const iv = hexToUint8(testVector.iv);
			const tag = hexToUint8(testVector.tag);
			const cipherText = hexToUint8(testVector.cipherText);
			const clearText = hexToUint8(testVector.clearText);

			// Act:
			const cipher = new AesGcmCipher(sharedKey);
			const resultCipherText = cipher.encrypt(clearText, iv);
			const resultClearText = cipher.decrypt(Uint8Array.of(...cipherText, ...tag), iv);

			// Assert:
			return [
				[Uint8Array.of(...cipherText, ...tag), resultCipherText],
				[clearText, resultClearText]
			];
		}
	}

	// endregion

	// region MosaicIdDerivationTester

	class MosaicIdDerivationTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(5, 'test-mosaic-id', 'mosaic id derivation');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const nonce = testVector.mosaicNonce;

			const mosaicIdPairs = [];
			['Public', 'PublicTest', 'Private', 'PrivateTest'].forEach(networkTag => {
				const address = new this.classLocator.Facade.Address(testVector[`address_${networkTag}`]);
				const expectedHexDigits = testVector[`mosaicId_${networkTag}`];
				const expectedMosaicId = BigInt(`0x${expectedHexDigits}`);

				// Act:
				const mosaicId = generateMosaicId(address, nonce);

				// Assert:
				mosaicIdPairs.push([expectedMosaicId, mosaicId]);
			});

			return mosaicIdPairs;
		}
	}

	// endregion

	// region Bip32DerivationTester, Bip39DerivationTester

	class Bip32DerivationTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(6, 'test-hd-derivation', 'BIP32 derivation');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const seed = hexToUint8(testVector.seed);
			const expectedRootPublicKey = new PublicKey(testVector.rootPublicKey);
			const expectedChildPublicKeys = testVector.childAccounts.map(childTestVector => new PublicKey(childTestVector.publicKey));

			// Act:
			const rootNode = new Bip32(this.classLocator.Facade.BIP32_CURVE_NAME).fromSeed(seed);
			const rootPublicKey = new this.classLocator.Facade.KeyPair(rootNode.privateKey).publicKey;

			const childPublicKeys = testVector.childAccounts.map(childTestVector => {
				const childNode = rootNode.derivePath(childTestVector.path);
				return this.classLocator.Facade.bip32NodeToKeyPair(childNode).publicKey;
			});

			// Assert:
			return [
				[expectedRootPublicKey, rootPublicKey],
				[expectedChildPublicKeys, childPublicKeys]
			];
		}
	}

	class Bip39DerivationTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(6, 'test-hd-derivation', 'BIP39 derivation');
			this.classLocator = classLocator;
		}

		process(testVector) {
			if (!testVector.mnemonic)
				return undefined;

			// Arrange:
			const { mnemonic, passphrase } = testVector;
			const expectedRootPublicKey = new PublicKey(testVector.rootPublicKey);

			// Act:
			const rootNode = new Bip32(this.classLocator.Facade.BIP32_CURVE_NAME).fromMnemonic(mnemonic, passphrase);
			const rootPublicKey = new this.classLocator.Facade.KeyPair(rootNode.privateKey).publicKey;

			// Assert:
			return [[expectedRootPublicKey, rootPublicKey]];
		}
	}

	// endregion

	// region VotingKeysGenerationTester

	class SeededPrivateKeyGenerator {
		constructor(values) {
			this.values = values;
			this.nextIndex = 0;
		}

		generate() {
			this.nextIndex += 1;
			return this.values[this.nextIndex - 1];
		}
	}

	class FibPrivateKeyGenerator {
		constructor(fillPrivateKey = false) {
			this.fillPrivateKey = fillPrivateKey;
			this.value1 = 1;
			this.value2 = 2;
		}

		generate() {
			const nextValue = this.value1 + this.value2;
			this.value1 = this.value2;
			this.value2 = nextValue;

			const seedValue = nextValue % 256;

			const privateKeyBuffer = new Uint8Array(PrivateKey.SIZE);
			if (this.fillPrivateKey) {
				for (let i = 0; i < PrivateKey.SIZE; ++i)
					privateKeyBuffer[i] = (seedValue + i) % 256;
			} else {
				privateKeyBuffer[PrivateKey.SIZE - 1] = seedValue;
			}

			return new PrivateKey(privateKeyBuffer);
		}
	}

	class VotingKeysGenerationTester extends VectorsTestSuite {
		constructor(classLocator) {
			super(7, 'test-voting-keys-generation', 'voting keys generation');
			this.classLocator = classLocator;
		}

		process(testVector) {
			// Arrange:
			const privateKeyGenerator = {
				test_vector_1: new FibPrivateKeyGenerator(),
				test_vector_2: new FibPrivateKeyGenerator(true),
				test_vector_3: new SeededPrivateKeyGenerator([
					new PrivateKey('12F98B7CB64A6D840931A2B624FB1EACAFA2C25C3EF0018CD67E8D470A248B2F'),
					new PrivateKey('B5593870940F28DAEE262B26367B69143AD85E43048D23E624F4ED8008C0427F'),
					new PrivateKey('6CFC879ABCCA78F5A4C9739852C7C643AEC3990E93BF4C6F685EB58224B16A59')
				])
			}[testVector.name];

			const rootPrivateKey = new PrivateKey(testVector.rootPrivateKey);
			const votingKeysGenerator = new VotingKeysGenerator(
				new this.classLocator.Facade.KeyPair(rootPrivateKey),
				() => privateKeyGenerator.generate()
			);

			// Act:
			const votingKeysBuffer = votingKeysGenerator.generate(BigInt(testVector.startEpoch), BigInt(testVector.endEpoch));

			// Assert:
			const expectedVotingKeysBuffer = hexToUint8(testVector.expectedFileHex);
			return [[expectedVotingKeysBuffer, votingKeysBuffer]];
		}
	}

	// endregion

	const loadClassLocator = blockchain => ('symbol' === blockchain
		? { Facade: SymbolFacade, Network: SymbolNetwork }
		: { Facade: NemFacade, Network: NemNetwork });

	const loadTestSuites = blockchain => {
		const classLocator = loadClassLocator(blockchain);
		const testSuites = [
			new KeyConversionTester(classLocator),
			new AddressConversionTester(classLocator),
			new SignTester(classLocator),
			new VerifyTester(classLocator),
			new DeriveTester(classLocator),
			new CipherTester(classLocator),
			new Bip32DerivationTester(classLocator),
			new Bip39DerivationTester(classLocator)
		];

		if ('symbol' === blockchain)
			[MosaicIdDerivationTester, VotingKeysGenerationTester].forEach(TesterClass => testSuites.push(new TesterClass(classLocator)));
		else
			[DeriveDeprecatedTester, CipherDeprecatedTester].forEach(TesterClass => testSuites.push(new TesterClass(classLocator)));

		return testSuites;
	};

	const areEqual = (lhs, rhs) => {
		if (!!lhs !== !!rhs)
			return false;

		if (lhs.length && rhs.length) {
			if (lhs.length !== rhs.length)
				return false;

			for (let i = 0; i < lhs.length; ++i) {
				if (!areEqual(lhs[i], rhs[i]))
					return false;
			}

			return true;
		}

		if (lhs.bytes && rhs.bytes)
			return areEqual(lhs.bytes, rhs.bytes);

		return lhs === rhs;
	};

	const hasAnyFailures = processedPairs => processedPairs.some(pair => !areEqual(pair[0], pair[1]));
	const args = yargs(process.argv.slice(2))
		.option('vectors', {
			describe: 'path to test-vectors directory',
			require: true,
			type: 'string'
		})
		.option('blockchain', {
			describe: 'blockchain to run vectors against',
			choices: ['nem', 'symbol'],
			default: 'symbol',
			type: 'string'
		})
		.option('tests', {
			describe: 'identifiers of tests to include',
			choices: [0, 1, 2, 3, 4, 5, 6, 7],
			array: true,
			type: 'array'
		})
		.parseSync();

	console.log(`running tests for ${args.blockchain} blockchain with vectors from ${args.vectors}`);

	let numFailedSuites = 0;
	const testSuites = loadTestSuites(args.blockchain);
	testSuites.forEach(testSuite => {
		if (args.tests && -1 === args.tests.indexOf(testSuite.identifier)) {
			console.log(`[ SKIPPED ] ${testSuite.description} test`);
			return;
		}

		let hasTestGroups = false;
		let testVectors = JSON.parse(fs.readFileSync(path.format({ dir: args.vectors, base: `${testSuite.filename}.json` })).toString());
		if (!Array.isArray(testVectors)) {
			hasTestGroups = true;
			testVectors = Object.keys(testVectors).map(name => ({
				testGroupName: name,
				testCases: testVectors[name]
			}));
		}

		const startTime = Date.now();

		let testCaseNumber = 0;
		let numFailed = 0;
		testVectors.forEach(testCaseOrGroup => {
			let testCases;
			if (hasTestGroups)
				({ testCases } = testCaseOrGroup);
			else
				testCases = [testCaseOrGroup];

			testCases.forEach(testCase => {
				const processedPairs = testSuite.process(testCase);
				if (!processedPairs)
					return;

				if (hasAnyFailures(processedPairs))
					++numFailed;

				++testCaseNumber;
			});
		});

		const elapsedTime = (Date.now() - startTime) / 1000;

		const testMessagePrefix = `[${elapsedTime.toFixed(4)}s] ${testSuite.description} test:`;
		if (numFailed) {
			console.log(`${testMessagePrefix} ${numFailed} failures out of ${testCaseNumber}`);
			++numFailedSuites;
		} else {
			console.log(`${testMessagePrefix} successes ${testCaseNumber}`);
		}
	});

	if (numFailedSuites)
		process.exit(1);
})();
