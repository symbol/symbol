const { Bip32 } = require('../src/Bip32');
const { PrivateKey, PublicKey, Signature } = require('../src/CryptoTypes');
const { NetworkLocator } = require('../src/Network');
const { generateMosaicId } = require('../src/symbol/idGenerator');
const { hexToUint8 } = require('../src/utils/converter');
const yargs = require('yargs');
const fs = require('fs');
const path = require('path');

(() => {
	// region TestSuites

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

	const loadClassLocator = blockchain => {
		if ('symbol' === blockchain) {
			const { SymbolFacade } = require('../src/facade/SymbolFacade'); // eslint-disable-line global-require
			const { Network } = require('../src/symbol/Network'); // eslint-disable-line global-require
			return { Facade: SymbolFacade, Network };
		}

		const { NemFacade } = require('../src/facade/NemFacade'); // eslint-disable-line global-require
		const { Network } = require('../src/nem/Network'); // eslint-disable-line global-require
		return { Facade: NemFacade, Network };
	};

	const loadTestSuites = blockchain => {
		const classLocator = loadClassLocator(blockchain);
		const testSuites = [
			new KeyConversionTester(classLocator),
			new AddressConversionTester(classLocator),
			new SignTester(classLocator),
			new VerifyTester(classLocator),
			new Bip32DerivationTester(classLocator),
			new Bip39DerivationTester(classLocator)
		];

		if ('symbol' === blockchain)
			testSuites.push(new MosaicIdDerivationTester(classLocator));

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
		.demandOption('vectors', 'path to test-vectors directory')
		.option('blockchain', {
			describe: 'blockchain to run vectors against',
			choices: ['nem', 'symbol'],
			default: 'symbol'
		})
		.option('tests', {
			describe: 'identifiers of tests to include',
			choices: [0, 1, 2, 3, 4, 5, 6],
			array: true
		})
		.argv;

	console.log(`running tests for ${args.blockchain} blockchain with vectors from ${args.vectors}`);

	let numFailedSuites = 0;
	const testSuites = loadTestSuites(args.blockchain);
	testSuites.forEach(testSuite => {
		if (args.tests && -1 === args.tests.indexOf(testSuite.identifier)) {
			console.log(`[ SKIPPED ] ${testSuite.description} test`);
			return;
		}

		let hasTestGroups = false;
		let testVectors = JSON.parse(fs.readFileSync(path.format({ dir: args.vectors, base: `${testSuite.filename}.json` })));
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
			let testGroupName;
			let testCases;
			if (hasTestGroups)
				({ testGroupName, testCases } = testCaseOrGroup);
			else
				testCases = [testCaseOrGroup];

			testCases.forEach(testCase => {
				const processedPairs = testSuite.process(testCase, testGroupName);
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
