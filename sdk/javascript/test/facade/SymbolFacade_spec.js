const { Bip32 } = require('../../src/Bip32');
const {
	Hash256, PrivateKey, PublicKey, Signature
} = require('../../src/CryptoTypes');
const { SymbolFacade } = require('../../src/facade/SymbolFacade');
const { expect } = require('chai');
const crypto = require('crypto');

describe('Symbol Facade', () => {
	// region constants

	it('has correct BIP32 constants', () => {
		expect(SymbolFacade.BIP32_COIN_ID).to.equal(4343);
		expect(SymbolFacade.BIP32_CURVE_NAME).to.equal('ed25519');
	});

	it('has correct KeyPair', () => {
		// Arrange:
		const privateKey = new PrivateKey('E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E');

		// Act:
		const keyPair = new SymbolFacade.KeyPair(privateKey);

		// Assert:
		expect(keyPair.publicKey).to.deep.equal(new PublicKey('E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD'));
	});

	it('can sign and verify', () => {
		// Arrange:
		const privateKey = PrivateKey.random();
		const keyPair = new SymbolFacade.KeyPair(privateKey);
		const message = new Uint8Array(crypto.randomBytes(21));

		// Act:
		const signature = keyPair.sign(message);
		const isVerified = new SymbolFacade.Verifier(keyPair.publicKey).verify(message, signature);

		// Assert:
		expect(isVerified).to.equal(true);
	});

	// endregion

	// region constructor

	it('can create around known network', () => {
		// Act:
		const facade = new SymbolFacade('testnet');
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction',
			signerPublicKey: new Uint32Array(PublicKey.SIZE)
		});

		// Assert:
		expect(facade.network.name).to.equal('testnet');
		expect(facade.network.identifier).to.equal(0x98);

		expect(transaction.type.value).to.equal(0x4154);
		expect(transaction.version).to.equal(1);
	});

	it('cannot create around unknown network', () => {
		expect(() => {
			new SymbolFacade('foo'); // eslint-disable-line no-new
		}).to.throw('no network found with name \'foo\'');
	});

	// endregion

	// region hash transaction / sign transaction

	const createRealTransfer = facade => facade.transactionFactory.create({
		type: 'transfer_transaction',
		signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
		fee: 1000000n,
		deadline: 41998024783n,
		recipientAddress: 'TD4PJKW5JP3CNHA47VDFIM25RCWTWRGT45HMPSA',
		mosaics: [
			{ mosaicId: 0x2CF403E85507F39En, amount: 1000000n }
		]
	});

	const createRealAggregate = facade => {
		const aggregate = facade.transactionFactory.create({
			type: 'aggregate_complete_transaction',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			fee: 2000000n,
			deadline: 42238390163n,
			transactionsHash: '71554638F578358B1D3FC4369AC625DB491AD5E5D4424D6DBED9FFC7411A37FE'
		});
		const transfer = facade.transactionFactory.createEmbedded({
			type: 'transfer_transaction',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			recipientAddress: 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
			mosaics: [
				{ mosaicId: 0x2CF403E85507F39En, amount: 1000000n }
			]
		});
		aggregate.transactions.push(transfer);
		return aggregate;
	};

	const assertCanHashTransaction = (transactionFactory, expectedHash) => {
		// Arrange:
		const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
		const facade = new SymbolFacade('testnet');

		const transaction = transactionFactory(facade);
		const signature = facade.signTransaction(new SymbolFacade.KeyPair(privateKey), transaction);
		facade.transactionFactory.constructor.attachSignature(transaction, signature);

		// Act:
		const hashValue = facade.hashTransaction(transaction);

		// Assert:
		expect(hashValue).to.deep.equal(expectedHash);
	};

	it('can hash transaction', () => {
		assertCanHashTransaction(createRealTransfer, new Hash256('17EBC7D64F01AA12F55A2B1F50C99B02BC25D06928CEAD1F249A4373B5EB1914'));
	});

	it('can hash aggregate transaction', () => {
		assertCanHashTransaction(createRealAggregate, new Hash256('A029FCAC4957C6531B4492F08C211CDDE52C3CD72F2016D6EA37EC96B85606E7'));
	});

	const assertCanSignTransaction = (transactionFactory, expectedSignature) => {
		// Arrange:
		const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
		const facade = new SymbolFacade('testnet');

		const transaction = transactionFactory(facade);

		// Sanity:
		expect(transaction.signature).to.deep.equal(Signature.zero());

		// Act:
		const signature = facade.signTransaction(new SymbolFacade.KeyPair(privateKey), transaction);

		// Assert:
		expect(expectedSignature).to.deep.equal(signature);
	};

	it('can sign transaction', () => {
		assertCanSignTransaction(createRealTransfer, new Signature([
			'9BC2691B3176149D5E76ED15D83BAB7AC403C754106DFA94E4264F73B92DEC1B',
			'1D514F23C07735EF394DA005AD96C86011EDF49F1FEE56CF3E280B49BEE26608'
		].join('')));
	});

	it('can sign aggregate transaction', () => {
		assertCanSignTransaction(createRealAggregate, new Signature([
			'CD95F7D677A66E980B0B24605049CF405CB1E350ACF65F2BC5427BBBFF531557',
			'487176A464DA6E5D6B17D71ADDD727C3D0C469513C1AB36F27547ED6101B4809'
		].join('')));
	});

	const assertCanVerifyTransaction = transactionFactory => {
		// Arrange:
		const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
		const facade = new SymbolFacade('testnet');

		const transaction = transactionFactory(facade);

		// Sanity:
		expect(transaction.signature).to.deep.equal(Signature.zero());

		// Act:
		const signature = facade.signTransaction(new SymbolFacade.KeyPair(privateKey), transaction);
		const isVerified = facade.verifyTransaction(transaction, signature);

		// Assert:
		expect(isVerified).to.equal(true);
	};

	it('can verify transaction', () => {
		assertCanVerifyTransaction(createRealTransfer);
	});

	it('can verify aggregate transaction', () => {
		assertCanVerifyTransaction(createRealAggregate);
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
		const rootNode = new Bip32(SymbolFacade.BIP32_CURVE_NAME).fromMnemonic(mnemonicSeed, passphrase);

		const childPublicKeys = [];
		for (let i = 0; i < expectedChildPublicKeys.length; ++i) {
			const childNode = rootNode.derivePath([44, SymbolFacade.BIP32_COIN_ID, i, 0, 0]);
			const childKeyPair = SymbolFacade.bip32NodeToKeyPair(childNode);
			childPublicKeys.push(childKeyPair.publicKey);
		}

		// Assert:
		expect(childPublicKeys).to.deep.equal(expectedChildPublicKeys);
	};

	it('can use BIP32 derivation without passphrase', () => {
		assertBip32ChildPublicKeys('', [
			new PublicKey('E9CFE9F59CB4393E61B2F42769D9084A644B16883C32C2823E7DF9A3AF83C121'),
			new PublicKey('0DE8C3235271E4C9ACF5482F7DFEC1E5C4B20FFC71548703EACF593153F116F9'),
			new PublicKey('259866A68A00C325713342232056333D60710E223FC920566B3248B266E899D5')
		]);
	});

	it('can use BIP32 derivation with passphrase', () => {
		assertBip32ChildPublicKeys('TREZOR', [
			new PublicKey('47F4D39D36D11C07735D7BE99220696AAEE7B3EE161D61422220DFE3FF120B15'),
			new PublicKey('4BA67E87E8C14F3EB82B3677EA959B56A9D7355705019CED1FCF6C76104E628C'),
			new PublicKey('8115D75C13C2D25E7FA3009D03D63F1F32601CDCCA9244D5FDAC74BCF3E892E3')
		]);
	});

	// endregion
});
