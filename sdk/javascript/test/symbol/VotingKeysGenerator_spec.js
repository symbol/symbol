const { PrivateKey, PublicKey, Signature } = require('../../src/CryptoTypes');
const { KeyPair, Verifier } = require('../../src/symbol/KeyPair');
const { VotingKeysGenerator } = require('../../src/symbol/VotingKeysGenerator');
const { hexToUint8, uint8ToHex } = require('../../src/utils/converter');
const { expect } = require('chai');
const YAML = require('yaml');
const fs = require('fs');
const path = require('path');

describe('VotingKeysGenerator', () => {
	// region basic test

	it('can generate header', () => {
		// Arrange:
		const rootKeyPair = new KeyPair(PrivateKey.random());
		const votingKeysGenerator = new VotingKeysGenerator(rootKeyPair);

		// Act:
		const votingKeysBuffer = votingKeysGenerator.generate(7n, 11n);

		// Assert:
		expect(votingKeysBuffer.length).to.equal(32 + PublicKey.SIZE + 16 + (5 * (PrivateKey.SIZE + Signature.SIZE)));

		const reader = new DataView(votingKeysBuffer.buffer);
		const headerOne = [0, 8, 16, 24].map(offset => reader.getBigUint64(offset, true));
		expect(headerOne).to.deep.equal([7n, 11n, 0xFFFFFFFFFFFFFFFFn, 0xFFFFFFFFFFFFFFFFn]);

		const headerRootPublicKey = votingKeysBuffer.subarray(32, 32 + PublicKey.SIZE);
		const headerTwo = [64, 72].map(offset => reader.getBigUint64(offset, true));
		expect(headerRootPublicKey).to.deep.equal(rootKeyPair.publicKey.bytes);
		expect(headerTwo).to.deep.equal([7n, 11n]);
	});

	it('can generate random child keys', () => {
		// Arrange:
		const rootKeyPair = new KeyPair(PrivateKey.random());
		const votingKeysGenerator = new VotingKeysGenerator(rootKeyPair);

		// Act:
		const votingKeysBuffer = votingKeysGenerator.generate(7n, 11n);

		// Assert:
		expect(votingKeysBuffer.length).to.equal(32 + PublicKey.SIZE + 16 + (5 * (PrivateKey.SIZE + Signature.SIZE)));

		const verifier = new Verifier(rootKeyPair.publicKey);
		for (let i = 0; 5 > i; ++i) {
			const startOffset = 80 + (96 * i);
			const childPrivateKey = new PrivateKey(votingKeysBuffer.subarray(startOffset, startOffset + PrivateKey.SIZE));
			const signature = new Signature(votingKeysBuffer.subarray(
				startOffset + PrivateKey.SIZE,
				startOffset + PrivateKey.SIZE + Signature.SIZE
			));

			const childKeyPair = new KeyPair(childPrivateKey);
			const signedPayload = new Uint8Array([...childKeyPair.publicKey.bytes, 11 - i, 0, 0, 0, 0, 0, 0, 0]);
			expect(verifier.verify(signedPayload, signature), `child at ${i}`).to.equal(true);
		}
	});

	// endregion

	describe('test vectors', () => {
		// region private key generators

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

		// endregion

		// region test vectors

		const printSideBySideBuffersDiff = (expectedBuffer, actualBuffer, batchSize, isQuiet = false) => {
			let numUnequalSegments = 0;
			for (let i = 0; i < (expectedBuffer.byteLength / batchSize) | 0; ++i) {
				const partStartOffset = i * batchSize;
				const partEndOffset = (i + 1) * batchSize;
				const expectedPart = expectedBuffer.subarray(partStartOffset, partEndOffset);
				const actualPart = actualBuffer.subarray(partStartOffset, partEndOffset);

				let areEqual = true;
				for (let j = 0; j < expectedPart.byteLength; ++j)
					areEqual = areEqual && expectedPart[j] === actualPart[j];

				const operator = areEqual ? '==' : '!=';
				if (!isQuiet)
					console.log(`E ${uint8ToHex(expectedPart)} ${operator} ${uint8ToHex(actualPart)}`); // eslint-disable-line no-console

				if (!areEqual)
					++numUnequalSegments;
			}

			return numUnequalSegments;
		};

		const runTestVector = (name, privateKeyGenerator) => {
			// Arrange:
			const testVectorsFilename = fs.readFileSync(path.format({
				dir: __dirname,
				base: 'resources/voting_keys_generator_test_vectors.yaml'
			}), { encoding: 'utf8' });
			const testVectors = YAML.parse(testVectorsFilename);
			const testVector = testVectors.find(testVectorI => name === testVectorI.name);

			const rootPrivateKey = new PrivateKey(testVector.root_private_key);
			const votingKeysGenerator = new VotingKeysGenerator(new KeyPair(rootPrivateKey), () => privateKeyGenerator.generate());

			// Act:
			const votingKeysBuffer = votingKeysGenerator.generate(BigInt(testVector.start_epoch), BigInt(testVector.end_epoch));

			// Assert:
			const expectedVotingKeysBuffer = hexToUint8(testVector.expected_file_hex);

			if (printSideBySideBuffersDiff(expectedVotingKeysBuffer, votingKeysBuffer, 16, true))
				printSideBySideBuffersDiff(expectedVotingKeysBuffer, votingKeysBuffer, 16);

			expect(votingKeysBuffer).to.deep.equal(expectedVotingKeysBuffer);
		};

		it('can generate test vector 1', () => {
			runTestVector('test_vector_1', new FibPrivateKeyGenerator());
		});

		it('can generate test vector 2', () => {
			runTestVector('test_vector_2', new FibPrivateKeyGenerator(true));
		});

		it('can generate test vector 3', () => {
			runTestVector('test_vector_3', new SeededPrivateKeyGenerator([
				new PrivateKey('12F98B7CB64A6D840931A2B624FB1EACAFA2C25C3EF0018CD67E8D470A248B2F'),
				new PrivateKey('B5593870940F28DAEE262B26367B69143AD85E43048D23E624F4ED8008C0427F'),
				new PrivateKey('6CFC879ABCCA78F5A4C9739852C7C643AEC3990E93BF4C6F685EB58224B16A59')
			]));
		});

		// endregion
	});
});
