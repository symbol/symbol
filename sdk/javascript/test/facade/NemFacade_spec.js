const { Bip32 } = require('../../src/Bip32');
const {
	Hash256, PrivateKey, PublicKey, Signature
} = require('../../src/CryptoTypes');
const { NemFacade } = require('../../src/facade/NemFacade');
const { Network } = require('../../src/nem/Network');
const { expect } = require('chai');
const crypto = require('crypto');

describe('NEM Facade', () => {
	// region constants

	it('has correct BIP32 constants', () => {
		expect(NemFacade.BIP32_CURVE_NAME).to.equal('ed25519-keccak');
	});

	it('has correct KeyPair', () => {
		// Arrange:
		const privateKey = new PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57'); // reversed

		// Act:
		const keyPair = new NemFacade.KeyPair(privateKey);

		// Assert:
		expect(keyPair.publicKey).to.deep.equal(new PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0'));
	});

	it('can sign and verify', () => {
		// Arrange:
		const privateKey = PrivateKey.random();
		const keyPair = new NemFacade.KeyPair(privateKey);
		const message = new Uint8Array(crypto.randomBytes(21));

		// Act:
		const signature = keyPair.sign(message);
		const isVerified = new NemFacade.Verifier(keyPair.publicKey).verify(message, signature);

		// Assert:
		expect(isVerified).to.equal(true);
	});

	// endregion

	// region constructor

	it('can create around known network by name', () => {
		// Act:
		const facade = new NemFacade('testnet');
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction',
			signerPublicKey: new Uint32Array(PublicKey.SIZE)
		});

		// Assert:
		expect(facade.network.name).to.equal('testnet');
		expect(facade.network.identifier).to.equal(0x98);

		expect(transaction.type.value).to.equal(0x0101);
		expect(transaction.version).to.equal(2);
	});

	it('cannot create around unknown network by name', () => {
		expect(() => {
			new NemFacade('foo'); // eslint-disable-line no-new
		}).to.throw('no network found with name \'foo\'');
	});

	it('can create around unknown network', () => {
		// Arrange:
		const network = new Network('foo', 0xDE);

		// Act:
		const facade = new NemFacade(network);
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction',
			signerPublicKey: new Uint32Array(PublicKey.SIZE)
		});

		// Assert:
		expect(facade.network.name).to.equal('foo');
		expect(facade.network.identifier).to.equal(0xDE);

		expect(transaction.type.value).to.equal(0x0101);
		expect(transaction.version).to.equal(2);
	});

	// endregion

	// region hash transaction / sign transaction

	const createRealTransfer = () => {
		const facade = new NemFacade('testnet');
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v1',
			signerPublicKey: 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
			fee: 0x186A0n,
			timestamp: 191205516,
			deadline: 191291916,
			recipientAddress: 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
			amount: 5100000n,
			messageEnvelopeSize: 0x11,
			message: {
				messageType: 'plain',
				message: 'blah blah'
			}
		});
		return transaction;
	};

	it('can hash transaction', () => {
		// Arrange:
		const facade = new NemFacade('testnet');

		const transaction = createRealTransfer();

		// Act:
		const hashValue = facade.hashTransaction(transaction);

		// Assert:
		expect(hashValue).to.deep.equal(new Hash256('A7064DB890A4E7329AAB2AE7DCFA5EC76D7E374590C61EC85E03C698DF4EA79D'));
	});

	it('can sign transaction', () => {
		// Arrange:
		const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
		const facade = new NemFacade('testnet');

		const transaction = createRealTransfer();

		// Sanity:
		expect(transaction.signature).to.deep.equal(Signature.zero());

		// Act:
		const signature = facade.signTransaction(new NemFacade.KeyPair(privateKey), transaction);

		// Assert:
		const expectedSignature = new Signature([
			'23A7B3433D16172E6C8659DB24233C5A8222C589098EA7A8FBBCB19691C67DB1',
			'3FB2AB7BB215265A3E3D74D32683516B03785BFEB2A2DE6DAC09F5E34A793706'
		].join(''));
		expect(expectedSignature).to.deep.equal(signature);
	});

	it('can verify transaction', () => {
		// Arrange:
		const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
		const facade = new NemFacade('testnet');

		const transaction = createRealTransfer();

		// Sanity:
		expect(transaction.signature).to.deep.equal(Signature.zero());

		// Act:
		const signature = facade.signTransaction(new NemFacade.KeyPair(privateKey), transaction);
		const isVerified = facade.verifyTransaction(transaction, signature);

		// Assert:
		expect(isVerified).to.equal(true);
	});

	// endregion

	// region bip32Path

	it('can construct proper BIP32 mainnet path', () => {
		// Arrange:
		const facade = new NemFacade('mainnet');

		// Act:
		const path = facade.bip32Path(2);

		// Act + Assert:
		expect(path).to.deep.equal([44, 43, 2, 0, 0]);
	});

	it('can construct proper BIP32 testnet path', () => {
		// Arrange:
		const facade = new NemFacade('testnet');

		// Act:
		const path = facade.bip32Path(2);

		// Act + Assert:
		expect(path).to.deep.equal([44, 1, 2, 0, 0]);
	});

	// endregion

	// region bip32NodeToKeyPair

	const assertBip32ChildPublicKeys = (passphrase, expectedChildPublicKeys) => {
		// Arrange:
		const mnemonicSeed = [
			'hamster', 'diagram', 'private', 'dutch', 'cause', 'delay', 'private', 'meat', 'slide', 'toddler', 'razor', 'book',
			'happy', 'fancy', 'gospel', 'tennis', 'maple', 'dilemma', 'loan', 'word', 'shrug', 'inflict', 'delay', 'length'
		].join(' ');

		// Act:
		const rootNode = new Bip32(NemFacade.BIP32_CURVE_NAME).fromMnemonic(mnemonicSeed, passphrase);

		const childPublicKeys = [];
		for (let i = 0; i < expectedChildPublicKeys.length; ++i) {
			const childNode = rootNode.derivePath(new NemFacade('mainnet').bip32Path(i));
			const childKeyPair = NemFacade.bip32NodeToKeyPair(childNode);
			childPublicKeys.push(childKeyPair.publicKey);
		}

		// Assert:
		expect(childPublicKeys).to.deep.equal(expectedChildPublicKeys);
	};

	it('can use BIP32 derivation without passphrase', () => {
		assertBip32ChildPublicKeys('', [
			new PublicKey('6C42BFAD2199CCB5C64E59868CC7A3F2AD29BDDCEB9754157DF136535E6B5EBA'),
			new PublicKey('782FF2375F75524106356092B4EE4BA098D28CF6571F1643867B9A11AEF509C6'),
			new PublicKey('20EEEFCAE026EBB3C3C51E9AF86A97AA146B34A5463CFE468B37C3CB49682408')
		]);
	});

	it('can use BIP32 derivation with passphrase', () => {
		assertBip32ChildPublicKeys('TREZOR', [
			new PublicKey('3BE4759796DD507D6E410CD8C65121E7E42DAC69699A9058E1F7663A390122CE'),
			new PublicKey('6B288C00800EC9FC0C30F35CEAFC2C5EC4066C2BE622822AAC70D67F215E5E6D'),
			new PublicKey('1AC159878D327E578C0130767E960C265753CAD5215FC992F1F71C41D00EADA3')
		]);
	});

	// endregion
});
