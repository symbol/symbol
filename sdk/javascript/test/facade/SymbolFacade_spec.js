import { Bip32 } from '../../src/Bip32.js';
import {
	Hash256, PrivateKey, PublicKey, Signature
} from '../../src/CryptoTypes.js';
import SymbolFacade from '../../src/facade/SymbolFacade.js';
import { Network } from '../../src/symbol/Network.js';
/* eslint-disable no-unused-vars */
import TransactionFactory from '../../src/symbol/TransactionFactory.js';
/* eslint-enable no-unused-vars */
import * as sc from '../../src/symbol/models.js';
import { expect } from 'chai';
import crypto from 'crypto';

describe('Symbol Facade', () => {
	// region real transactions

	const createRealTransfer = facade => facade.transactionFactory.create({
		type: 'transfer_transaction_v1',
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
			type: 'aggregate_complete_transaction_v1',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			fee: 2000000n,
			deadline: 42238390163n,
			transactionsHash: '71554638F578358B1D3FC4369AC625DB491AD5E5D4424D6DBED9FFC7411A37FE'
		});
		const transfer = facade.transactionFactory.createEmbedded({
			type: 'transfer_transaction_v1',
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
			type: 'transfer_transaction_v1',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			recipientAddress: 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
			mosaics: [
				{ mosaicId: 0x2CF403E85507F39En, amount: 1000000n }
			]
		},
		{
			type: 'secret_proof_transaction_v1',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			recipientAddress: 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y',
			secret: 'BE254D2744329BBE20F9CF6DA61043B4CEF8C2BC000000000000000000000000',
			hashAlgorithm: 'hash_256',
			proof: '41FB'
		},
		{
			type: 'address_alias_transaction_v1',
			signerPublicKey: '87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8',
			namespaceId: 0xA95F1F8A96159516n,
			address: 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y',
			aliasAction: 'link'
		}
	].map(descriptor => facade.transactionFactory.createEmbedded(descriptor));

	const createRealAggregateSwap = facade => facade.transactionFactory.create({
		type: 'aggregate_complete_transaction_v1',
		signerPublicKey: '4C94E8B0A1DAB8573BCB6632E676F742E0D320FC8102F20FB7FB13BCAE9A9F60',
		fee: 36000n,
		deadline: 26443750218n,
		transactionsHash: '641CB7E431F1D44094A43E1CE8265E6BD1DF1C3B0B64797CDDAA0A375FCD3C08',
		transactions: [
			facade.transactionFactory.createEmbedded({
				type: 'transfer_transaction_v1',
				signerPublicKey: '29856F43A5C4CBDE42F2FAC775A6F915E9E5638CF458E9352E7B410B662473A3',
				recipientAddress: 'TBEZ3VKFBMKQSW7APBVL5NWNBEU7RR466PRRTDQ',
				mosaics: [
					{ mosaicId: 0xE74B99BA41F4AFEEn, amount: 20000000n }
				]
			}),
			facade.transactionFactory.createEmbedded({
				type: 'transfer_transaction_v1',
				signerPublicKey: '4C94E8B0A1DAB8573BCB6632E676F742E0D320FC8102F20FB7FB13BCAE9A9F60',
				recipientAddress: 'TDFR3Q3H5W4OPOSHALVDY3RF4ZQNH44LIUIHYTQ',
				mosaics: [
					{ mosaicId: 0x798A29F48E927C83n, amount: 100n }
				]
			})
		]
	});

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
			type: 'transfer_transaction_v1',
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
		const network = new Network('foo', 0xDE, new Date(), Hash256.zero());

		// Act:
		const facade = new SymbolFacade(network);
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v1',
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

	const attachSignature = (facade, transaction, signature) => {
		(/** @type typeof TransactionFactory */(facade.transactionFactory.constructor)).attachSignature(transaction, signature);
	};

	const assertCanHashTransaction = (transactionFactory, expectedHash) => {
		// Arrange:
		const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
		const facade = new SymbolFacade('testnet');

		const transaction = transactionFactory(facade);
		const signature = facade.signTransaction(new SymbolFacade.KeyPair(privateKey), transaction);
		attachSignature(facade, transaction, signature);

		// Act:
		const hashValue = facade.hashTransaction(transaction);

		// Assert:
		expect(hashValue).to.deep.equal(expectedHash);
	};

	it('can hash transaction', () => {
		assertCanHashTransaction(createRealTransfer, new Hash256('86E006F0D400A781A15D0293DFC15897078351A2F7731D49A865A63C2010DE44'));
	});

	it('can hash aggregate transaction', () => {
		assertCanHashTransaction(createRealAggregate, new Hash256('D074716D62F4CDF1CE219D7E0580DC2C030102E216ECE2037FA28A3BC5726BD0'));
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
			'24A3788AFD0223083D47ED14F17A2499A7939CD62C4B3288C40CF2736B13F404',
			'8486680DD574C9F7DB56F453464058CB22349ACBFAECAE16A31EF0725FFF6104'
		].join('')));
	});

	it('can sign aggregate transaction', () => {
		assertCanSignTransaction(createRealAggregate, new Signature([
			'40C5C9F0BAF74E64877982C411D0D16665E18D463B66204081D846564FC6CAE1',
			'3F1F75C688CBD2D34263DA166537A90B4F371C1B38DDF00414AB0F5D78C3CD0F'
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

	// region cosignTransaction

	describe('can cosign transactions', () => {
		const assertCanCosignTransaction = detached => {
			// Arrange:
			const signerPrivateKey = new PrivateKey('F4BC233E183E8CEA08D0A604A3DC67FF3261D1E6EBF84D233488BC53D89C50B7');
			const cosignerPrivateKey = new PrivateKey('BE7B98F835A896136ADDAF04220F28CB4925D24F0675A21421BF213C180BEF86');
			const facade = new SymbolFacade('testnet');

			const transaction = createRealAggregateSwap(facade);
			const signature = facade.signTransaction(new SymbolFacade.KeyPair(signerPrivateKey), transaction);
			attachSignature(facade, transaction, signature);

			// Act:
			const cosignature = facade.cosignTransaction(new SymbolFacade.KeyPair(cosignerPrivateKey), transaction, detached);

			// Assert: check common fields
			const expectedPublicKeyBytes = new PublicKey('29856F43A5C4CBDE42F2FAC775A6F915E9E5638CF458E9352E7B410B662473A3').bytes;
			const expectedSignatureBytes = new Signature('204BD2C4F86B66313E5C5F817FD650B108826D53EDEFC8BDFF936E4D6AA07E38'
					+ '5F819CF0BF22D14D4AA2011AD07BC0FE6023E2CB48DC5D82A6A1FF1348FA3E0B').bytes;

			expect(cosignature.version).to.equal(0n);
			expect(cosignature.signerPublicKey).to.deep.equal(new sc.PublicKey(expectedPublicKeyBytes));
			expect(cosignature.signature).to.deep.equal(new sc.Signature(expectedSignatureBytes));
			return cosignature;
		};

		it('as attached cosignature', () => {
			// Act:
			const cosignature = assertCanCosignTransaction();

			// Assert: cosignature should be suitable for attaching to an aggregate
			expect(cosignature.size).to.equal(104);
			expect(Object.prototype.hasOwnProperty.call(cosignature, '_parentHash')).to.equal(false);
		});

		it('as detached cosignature', () => {
			// Act:
			const cosignature = /** @type {sc.DetachedCosignature} */ (assertCanCosignTransaction(true));

			// Assert: cosignature should be detached
			const expectedHashBytes = new Hash256('214DFF47469D462E1D9A03232C2582C7E44DE026A287F98529CC74DE9BD69641').bytes;

			expect(cosignature.size).to.equal(136);
			expect(Object.prototype.hasOwnProperty.call(cosignature, '_parentHash')).to.equal(true);
			expect(cosignature.parentHash).to.deep.equal(new sc.Hash256(expectedHashBytes));
		});
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
