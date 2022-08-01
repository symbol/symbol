const { Hash256 } = require('../../src/CryptoTypes');
const { MerkleHashBuilder, proveMerkle } = require('../../src/symbol/merkle');
const { expect } = require('chai');
const crypto = require('crypto');

describe('merkle', () => {
	// region MerkleHashBuilder

	describe('MerkleHashBuilder', () => {
		const calculateMerkleHash = seedHashes => {
			const builder = new MerkleHashBuilder();
			seedHashes.forEach(seedHash => { builder.update(seedHash); });
			return builder.final();
		};

		it('can build from zero hashes', () => {
			// Act:
			const merkleHash = calculateMerkleHash([]);

			// Assert:
			expect(merkleHash).to.deep.equal(Hash256.zero());
		});

		it('can build from one hash', () => {
			// Arrange:
			const seedHash = new Hash256(crypto.randomBytes(Hash256.SIZE));

			// Act:
			const merkleHash = calculateMerkleHash([seedHash]);

			// Assert:
			expect(merkleHash).to.deep.equal(seedHash);
		});

		it('can build from balanced tree', () => {
			// Act:
			const merkleHash = calculateMerkleHash([
				new Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8'),
				new Hash256('8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28'),
				new Hash256('6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7'),
				new Hash256('2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681'),
				new Hash256('421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE'),
				new Hash256('7A1711AF5C402CFEFF87F6DA4B9C738100A7AC3EDAD38D698DF36CA3FE883480'),
				new Hash256('1E6516B2CC617E919FAE0CF8472BEB2BFF598F19C7A7A7DC260BC6715382822C'),
				new Hash256('410330530D04A277A7C96C1E4F34184FDEB0FFDA63563EFD796C404D7A6E5A20')
			]);

			// Assert:
			expect(merkleHash).to.deep.equal(new Hash256('7D853079F5F9EE30BDAE49C4956AF20CDF989647AFE971C069AC263DA1FFDF7E'));
		});

		it('can build from unbalanced tree', () => {
			// Act:
			const merkleHash = calculateMerkleHash([
				new Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8'),
				new Hash256('8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28'),
				new Hash256('6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7'),
				new Hash256('2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681'),
				new Hash256('421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE')
			]);

			// Assert:
			expect(merkleHash).to.deep.equal(new Hash256('DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09'));
		});

		it('produces different merkle hash when sub hash order changes', () => {
			// Arrange:
			const seedHashes1 = Array.from({ length: 8 }, () => new Hash256(crypto.randomBytes(Hash256.SIZE)));
			const seedHashes2 = Array.from([0, 1, 2, 5, 4, 3, 6, 7], value => seedHashes1[value]);

			// Sanity:
			expect(seedHashes2.length).to.equal(seedHashes1.length);

			// Act:
			const merkleHash1 = calculateMerkleHash(seedHashes1);
			const merkleHash2 = calculateMerkleHash(seedHashes2);

			// Assert:
			expect(merkleHash2).to.not.deep.equal(merkleHash1);
		});

		it('produces different merkle hash when sub hash changes', () => {
			// Arrange:
			const seedHashes1 = Array.from({ length: 8 }, () => new Hash256(crypto.randomBytes(Hash256.SIZE)));
			const seedHashes2 = Array.from(
				[0, 1, 2, 3, -1, 5, 6, 7],
				value => (0 <= value ? seedHashes1[value] : new Hash256(crypto.randomBytes(Hash256.SIZE)))
			);

			// Act:
			const merkleHash1 = calculateMerkleHash(seedHashes1);
			const merkleHash2 = calculateMerkleHash(seedHashes2);

			// Assert:
			expect(merkleHash2).to.not.deep.equal(merkleHash1);
		});
	});

	// endregion

	// region proveMerkle

	describe('proveMerkle', () => {
		it('succeeds when leaf is root and there is no path', () => {
			// Arrange:
			const rootHash = new Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8');

			// Act:
			const result = proveMerkle(rootHash, [], rootHash);

			// Assert:
			expect(result).to.equal(true);
		});

		it('fails when leaf is root and there is path', () => {
			// Arrange:
			const rootHash = new Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8');
			const merklePath = [
				{ hash: new Hash256('6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7'), isLeft: true }
			];

			// Act:
			const result = proveMerkle(rootHash, merklePath, rootHash);

			// Assert:
			expect(result).to.equal(false);
		});

		const createDefaultTestVector = () => ({
			leafHash: new Hash256('D4713ABB76AC98FB74AB91607C9029A95821C28462DC43707D92DD35E10B96CD'),
			merklePath: [
				{ hash: new Hash256('2CFB84D7A2F53FFAE07B1A686D84CB2491AD234F785B9C5905F1FF04E921F3F7'), isLeft: false },
				{ hash: new Hash256('B49544CFA100301340F7F060C935B02687041431BC660E288176B1954D5DF5D0'), isLeft: false },
				{ hash: new Hash256('0C346E96C61C4E54BCC10F1A4604C30C4A6D1E51691385BFFF2B9E56B2E0A9EB'), isLeft: false },
				{ hash: new Hash256('399887ED3F5C3086A1DFF78B78697C1592E2C35C10FB45B5AAF621AB52D23B78'), isLeft: true },
				{ hash: new Hash256('55DFB13E3F549DA89E9C38C97E7D2A557EE9B660EDA10DBD2088FB4CAFEF2524'), isLeft: false },
				{ hash: new Hash256('190C27BF7B21C474E99E3FE0F3DBF437F33C784D80732C2F8263D3F6A0167C58'), isLeft: false },
				{ hash: new Hash256('0B7DC05FA282E3BB156EE2861DF7E456AF538D56B4452996C39B4F5E46E2233E'), isLeft: false },
				{ hash: new Hash256('60E4C788A881D84AFEA758C660E9C779A460F022AE3EC38D584155F08E84D82E'), isLeft: true }
			],
			rootHash: new Hash256('DDDBA0604EE6A2CB9825CA5E0D31785F05F43713C5E7C512A900A7287DB5143C')
		});

		it('succeeds when proof is valid', () => {
			// Arrange:
			const testVector = createDefaultTestVector();

			// Act:
			const result = proveMerkle(testVector.leafHash, testVector.merklePath, testVector.rootHash);

			// Assert:
			expect(result).to.equal(true);
		});

		it('fails when root does not match', () => {
			// Arrange:
			const testVector = createDefaultTestVector();
			testVector.rootHash.bytes[0] ^= 0xFF;

			// Act:
			const result = proveMerkle(testVector.leafHash, testVector.merklePath, testVector.rootHash);

			// Assert:
			expect(result).to.equal(false);
		});

		it('fails when branch position is wrong', () => {
			// Arrange:
			const testVector = createDefaultTestVector();
			testVector.merklePath[4].isLeft = !testVector.merklePath[4].isLeft;

			// Act:
			const result = proveMerkle(testVector.leafHash, testVector.merklePath, testVector.rootHash);

			// Assert:
			expect(result).to.equal(false);
		});
	});

	// endregion
});
