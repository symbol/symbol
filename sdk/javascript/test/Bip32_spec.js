import { Bip32 } from '../src/Bip32.js';
import { PrivateKey } from '../src/CryptoTypes.js';
import { hexToUint8 } from '../src/utils/converter.js';
import { expect } from 'chai';

const DETERIMINISTIC_SEED = hexToUint8('000102030405060708090A0B0C0D0E0F');
const DETERIMINISTIC_MNEMONIC = 'cat swing flag economy stadium alone churn speed unique patch report train';

describe('Bip32', () => {
	const assertBip32Node = (node, expectedChainCode, expectedPrivateKey) => {
		expect(node.chainCode).to.deep.equal(expectedChainCode);
		expect(node.privateKey).to.deep.equal(new PrivateKey(expectedPrivateKey));
	};

	describe('fromSeed', () => {
		it('can create root node', () => {
			// Act:
			const node = new Bip32().fromSeed(DETERIMINISTIC_SEED);

			// Assert:
			assertBip32Node(
				node,
				hexToUint8('90046A93DE5380A72B5E45010748567D5EA02BBF6522F979E05C0D8D8CA9FFFB'),
				hexToUint8('2B4BE7F19EE27BBF30C667B642D5F4AA69FD169872F8FC3059C08EBAE2EB19E7')
			);
		});

		it('can create root node with custom curve name', () => {
			// Act:
			const node = new Bip32('ed25519-keccak').fromSeed(DETERIMINISTIC_SEED);

			// Assert:
			assertBip32Node(
				node,
				hexToUint8('9CFCA256458AAC0A0550A30DC7639D87364E4323BA61ED41454818E3317BAED0'),
				hexToUint8('A3D76D92ACF784D68F4EA2F6DE5507A3520385237A80277132B6C8F3685601B2')
			);
		});

		it('can derive single level node', () => {
			// Act:
			const node = new Bip32().fromSeed(DETERIMINISTIC_SEED).deriveOne(0);

			// Assert:
			assertBip32Node(
				node,
				hexToUint8('8B59AA11380B624E81507A27FEDDA59FEA6D0B779A778918A2FD3590E16E9C69'),
				hexToUint8('68E0FE46DFB67E368C75379ACEC591DAD19DF3CDE26E63B93A8E704F1DADE7A3')
			);
		});

		const assertWellKnownChildrenFromDeterministicSeed = (childNode0, childNode1) => {
			assertBip32Node(
				childNode0,
				hexToUint8('B8E16D407C8837B46A9445C6417310F3C7A4DCD9B8FF2679C383E6DEF721AC11'),
				hexToUint8('BB2724A538CFD64E4366FEB36BB982B954D58EA78F7163451B3B514EDD692159')
			);
			assertBip32Node(
				childNode1,
				hexToUint8('68CA2A058611AAC20CAFB4E1CCD70961E67D8C567390B3CBFC63D0E58BAE7153'),
				hexToUint8('8C91D9F5D214A2E80A275E75A165F7022712F7AD52B7ECD45B3B6CC76154B571')
			);
		};

		it('can derive well known child nodes (chained)', () => {
			// Act:
			const node = new Bip32().fromSeed(DETERIMINISTIC_SEED);
			const childNode0 = node
				.deriveOne(44)
				.deriveOne(4343)
				.deriveOne(0)
				.deriveOne(0)
				.deriveOne(0);
			const childNode1 = node
				.deriveOne(44)
				.deriveOne(4343)
				.deriveOne(1)
				.deriveOne(0)
				.deriveOne(0);

			// Assert:
			assertWellKnownChildrenFromDeterministicSeed(childNode0, childNode1);
		});

		it('can derive well known child nodes (path)', () => {
			// Act:
			const node = new Bip32().fromSeed(DETERIMINISTIC_SEED);
			const childNode0 = node.derivePath([44, 4343, 0, 0, 0]);
			const childNode1 = node.derivePath([44, 4343, 1, 0, 0]);

			// Assert:
			assertWellKnownChildrenFromDeterministicSeed(childNode0, childNode1);
		});
	});

	describe('fromMnemonic', () => {
		it('can derive child nodes from mnemonic with password', () => {
			// Act:
			const node = new Bip32().fromMnemonic(DETERIMINISTIC_MNEMONIC, 'TREZOR');
			const childNode0 = node.derivePath([44, 4343, 0, 0, 0]);
			const childNode1 = node.derivePath([44, 4343, 1, 0, 0]);
			const childNode2 = node.derivePath([44, 4343, 2, 0, 0]);

			// Assert:
			expect(childNode0.privateKey).to.deep.equal(new PrivateKey('1455FB18AB105444763EED593B7CA1C53EF6DDF8BDA1AB7004276FEAB1FCF222'));
			expect(childNode1.privateKey).to.deep.equal(new PrivateKey('913967B3DFE1E94C50D5C92789DA194644D2A699E5BB75B171A3B68993B82A21'));
			expect(childNode2.privateKey).to.deep.equal(new PrivateKey('AEC7C0143FC11F26FF5DB020492DACA7C8CF2640D2377AD3C721286472571602'));
		});
	});

	describe('random', () => {
		it('can create random mnemonic with default seed length', () => {
			// Act:
			const mnemonic1 = new Bip32().random();
			const mnemonic2 = new Bip32().random();

			// Assert:
			expect(mnemonic1.split(' ').length).to.equal(24);
			expect(mnemonic2.split(' ').length).to.equal(24);
			expect(mnemonic1).to.not.equal(mnemonic2);
		});

		it('can create random mnemonic with custom seed length', () => {
			// Act:
			const mnemonic1 = new Bip32().random(16);
			const mnemonic2 = new Bip32().random(16);

			// Assert:
			expect(mnemonic1.split(' ').length).to.equal(12);
			expect(mnemonic2.split(' ').length).to.equal(12);
			expect(mnemonic1).to.not.equal(mnemonic2);
		});

		it('cannot create random mnemonic with invalid seed length', () => {
			// Act + Assert:
			expect(() => { new Bip32().random(18); }).to.throw('Invalid Argument: ENT Documentation');
		});
	});
});
