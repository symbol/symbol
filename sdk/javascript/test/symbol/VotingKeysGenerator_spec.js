import { PrivateKey, PublicKey, Signature } from '../../src/CryptoTypes.js';
import { KeyPair, Verifier } from '../../src/symbol/KeyPair.js';
import VotingKeysGenerator from '../../src/symbol/VotingKeysGenerator.js';
import { expect } from 'chai';

describe('VotingKeysGenerator', () => {
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
});
