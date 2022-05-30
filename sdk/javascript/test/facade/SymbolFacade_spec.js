const { Bip32 } = require('../../src/Bip32');
const {
	Hash256, PrivateKey, PublicKey, Signature
} = require('../../src/CryptoTypes');
const { SymbolFacade } = require('../../src/facade/SymbolFacade');
const { Network } = require('../../src/symbol/Network');
const { expect } = require('chai');
const crypto = require('crypto');

describe('Symbol Facade', () => {
	// region real transactions

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

	const createRealEmbeddedTransactions = facade => [
		{
			type: 'transfer_transaction',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			recipientAddress: 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
			mosaics: [
				{ mosaicId: 0x2CF403E85507F39En, amount: 1000000n }
			]
		},
		{
			type: 'secret_proof_transaction',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			recipientAddress: 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y',
			secret: 'BE254D2744329BBE20F9CF6DA61043B4CEF8C2BC000000000000000000000000',
			hashAlgorithm: 'hash_256',
			proof: '41FB'
		},
		{
			type: 'address_alias_transaction',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			namespaceId: 0xA95F1F8A96159516n,
			address: 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y',
			aliasAction: 'link'
		}
	].map(descriptor => facade.transactionFactory.createEmbedded(descriptor));

	// endregion

	// region constants

	it('has correct BIP32 constants', () => {
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

	it('can create around known network by name', () => {
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

	it('cannot create around unknown network by name', () => {
		expect(() => {
			new SymbolFacade('foo'); // eslint-disable-line no-new
		}).to.throw('no network found with name \'foo\'');
	});

	it('can create around unknown network', () => {
		// Arrange:
		const network = new Network('foo', 0xDE);

		// Act:
		const facade = new SymbolFacade(network);
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction',
			signerPublicKey: new Uint32Array(PublicKey.SIZE)
		});

		// Assert:
		expect(facade.network.name).to.equal('foo');
		expect(facade.network.identifier).to.equal(0xDE);

		expect(transaction.type.value).to.equal(0x4154);
		expect(transaction.version).to.equal(1);
	});

	// endregion

	// region hash transaction / sign transaction

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
		assertCanHashTransaction(createRealTransfer, new Hash256('600D0CF8C95CDEEB1BC81EFEB9D50BB853F474AC2226E1BEB83E235716C8E16E'));
	});

	it('can hash aggregate transaction', () => {
		assertCanHashTransaction(createRealAggregate, new Hash256('194578BACECBE33A18EE6D1BE02D61B1CC86F57D57C4D22F7783D27EB33FF225'));
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
			'5BF0C9DC0D97FDE7FF6F99F1EFADF50DD77C1FA54CFC704FB23295C8F6908B6D',
			'1F9BA1FB2DB267543805F14C83B7A9D4255D8AECC6046DDBE225115A6DF16002'
		].join('')));
	});

	it('can sign aggregate transaction', () => {
		assertCanSignTransaction(createRealAggregate, new Signature([
			'116BA7B83280BC1752440A5CFBF71612385DFDFA0363A5B220E20C0CA0C6307A',
			'35C979BB120BAB85E58B1C880DDFB7A96A922D1A2828B5C6CC9556C27571190C'
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

	// region hashEmbeddedTransactions

	it('can hash embedded transactions', () => {
		// Arrange:
		const facade = new SymbolFacade('testnet');
		const transaction = createRealAggregate(facade);

		// Act:
		const hashValue = SymbolFacade.hashEmbeddedTransactions(transaction.transactions);

		// Assert:
		expect(hashValue).to.deep.equal(transaction.transactionsHash);
	});

	it('can hash embedded transactions (multiple)', () => {
		// Arrange:
		const facade = new SymbolFacade('testnet');
		const transactions = createRealEmbeddedTransactions(facade);

		// Act:
		const hashValue = SymbolFacade.hashEmbeddedTransactions(transactions);

		// Assert:
		expect(hashValue).to.deep.equal(new Hash256('5C78999F21EA75B880100E1B4C76166B9C320869F67C00D28F9F8F754D7831C9'));
	});

	// endregion

	// region bip32Path

	it('can construct proper BIP32 mainnet path', () => {
		// Arrange:
		const facade = new SymbolFacade('mainnet');

		// Act:
		const path = facade.bip32Path(2);

		// Act + Assert:
		expect(path).to.deep.equal([44, 4343, 2, 0, 0]);
	});

	it('can construct proper BIP32 testnet path', () => {
		// Arrange:
		const facade = new SymbolFacade('testnet');

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
		const rootNode = new Bip32(SymbolFacade.BIP32_CURVE_NAME).fromMnemonic(mnemonicSeed, passphrase);

		const childPublicKeys = [];
		for (let i = 0; i < expectedChildPublicKeys.length; ++i) {
			const childNode = rootNode.derivePath(new SymbolFacade('mainnet').bip32Path(i));
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
