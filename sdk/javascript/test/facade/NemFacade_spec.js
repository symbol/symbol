import { Bip32 } from '../../src/Bip32.js';
import {
	Hash256, PrivateKey, PublicKey, Signature
} from '../../src/CryptoTypes.js';
import { NemFacade } from '../../src/facade/NemFacade.js';
import { Address, Network } from '../../src/nem/Network.js';
import TransactionFactory from '../../src/nem/TransactionFactory.js';
import * as nc from '../../src/nem/models.js';
import * as descriptors from '../../src/nem/models_ts.js';
import { expect } from 'chai';
import crypto from 'crypto';

describe('NEM Facade', () => {
	// region real transactions

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
			message: {
				messageType: 'plain',
				message: 'blah blah'
			}
		});
		return transaction;
	};

	const createRealMultisigTransaction = () => {
		const facade = new NemFacade('testnet');
		const innerTransaction = TransactionFactory.toNonVerifiableTransaction(createRealTransfer());

		const transaction = facade.transactionFactory.create({
			type: 'multisig_transaction_v1',
			signerPublicKey: 'A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74',
			fee: 0x123456n,
			timestamp: 191205516,
			deadline: 191291916,

			innerTransaction
		});
		return transaction;
	};

	// endregion

	// region constants

	it('has correct BIP32 constants', () => {
		expect(NemFacade.BIP32_CURVE_NAME).to.equal('ed25519-keccak');
	});

	it('has correct static accessor', () => {
		// Arrange:
		const facade = new NemFacade('testnet');

		// Assert:
		expect(NemFacade).to.deep.equal(facade.static);
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
			type: 'transfer_transaction_v2',
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
		const network = new Network('foo', 0xDE, new Date());

		// Act:
		const facade = new NemFacade(network);
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v2',
			signerPublicKey: new Uint32Array(PublicKey.SIZE)
		});

		// Assert:
		expect(facade.network.name).to.equal('foo');
		expect(facade.network.identifier).to.equal(0xDE);

		expect(transaction.type.value).to.equal(0x0101);
		expect(transaction.version).to.equal(2);
	});

	// endregion

	// region now

	it('can create current timestamp for network via now', () => {
		for (;;) {
			// Arrange: affinitize test to run so that whole test runs within the context of the same millisecond
			const startTime = new Date().getTime();
			const facade = new NemFacade('testnet');

			// Act:
			const nowFromFacade = facade.now();
			const nowFromNetwork = facade.network.fromDatetime(new Date(Date.now()));

			const endTime = new Date().getTime();
			if (startTime !== endTime)
				continue; // eslint-disable-line no-continue

			// Assert:
			expect(nowFromFacade).to.deep.equal(nowFromNetwork);
			expect(0n < nowFromFacade.timestamp).to.equal(true);
			break;
		}
	});

	// endregion

	// region createPublicAccount / createAccount

	describe('account wrappers', () => {
		it('can create public account from public key', () => {
			// Arrange:
			const facade = new NemFacade('testnet');
			const publicKey = new PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0');

			// Act:
			const account = facade.createPublicAccount(publicKey);

			// Assert:
			expect(account.address).to.deep.equal(new Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33'));
			expect(account.publicKey).to.deep.equal(publicKey);
		});

		it('can create account from private key', () => {
			// Arrange:
			const facade = new NemFacade('testnet');
			const publicKey = new PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0');
			const privateKey = new PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57');

			// Act:
			const account = facade.createAccount(privateKey);

			// Assert:
			expect(account.address).to.deep.equal(new Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33'));
			expect(account.publicKey).to.deep.equal(publicKey);
			expect(account.keyPair.publicKey).to.deep.equal(publicKey);
			expect(account.keyPair.privateKey).to.deep.equal(privateKey);
		});

		it('can create message encoder', () => {
			// Arrange:
			const facade = new NemFacade('testnet');
			const account = facade.createAccount(new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC'));

			// Act:
			const encoder = account.messageEncoder();

			// Assert: message encoder matches the account
			expect(encoder.publicKey).to.deep.equal(account.publicKey);
		});

		it('can sign and verify transaction', () => {
			// Arrange:
			const facade = new NemFacade('testnet');
			const account = facade.createAccount(new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC'));

			const transaction = createRealTransfer();

			// Sanity:
			expect(transaction.signature).to.deep.equal(Signature.zero());

			// Act:
			const signature = account.signTransaction(transaction);
			const isVerified = facade.verifyTransaction(transaction, signature);

			// Assert:
			expect(isVerified).to.equal(true);
		});
	});

	// endregion

	// region create from typed descriptor

	it('can create transaction from typed descriptor', () => {
		// Arrange:
		const facade = new NemFacade('testnet');
		const nowTimestamp = facade.now();

		const signerPublicKey = new PublicKey('87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8');
		const typedDescriptor = new descriptors.TransferTransactionV1Descriptor(
			new Address('TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW'),
			new nc.Amount(1000000n),
			new descriptors.MessageDescriptor(nc.MessageType.PLAIN, 'hello nem')
		);

		// Act:
		const transaction = (/** @type {nc.TransferTransactionV1} */ (facade.createTransactionFromTypedDescriptor(
			typedDescriptor,
			signerPublicKey,
			100n,
			60 * 60
		)));

		// Assert:
		expect(transaction.type).to.equal(nc.TransactionType.TRANSFER);
		expect(transaction.version).to.equal(1);
		expect(transaction.network).to.deep.equal(nc.NetworkType.TESTNET);
		expect(transaction.message.message).to.deep.equal(new TextEncoder().encode('hello nem'));

		expect(transaction.signerPublicKey).to.deep.equal(signerPublicKey);
		expect(transaction.fee.value).to.equal(100n);

		// - check timestamp and deadline are in range (within 10s)
		expect(nowTimestamp.timestamp <= transaction.timestamp.value).to.equal(true);
		expect(transaction.timestamp.value <= (nowTimestamp.timestamp + 10n)).to.equal(true);

		const minRawDeadline = nowTimestamp.timestamp + (60n * 60n);
		expect(minRawDeadline <= transaction.deadline.value).to.equal(true);
		expect(transaction.deadline.value <= (minRawDeadline + 10n)).to.equal(true);
	});

	// endregion

	// region hash transaction / sign transaction

	const addTransactionTests = descriptor => {
		it(`can hash ${descriptor.name} transaction`, () => {
			// Arrange:
			const facade = new NemFacade('testnet');

			const transaction = descriptor.createTransaction();

			// Act:
			const hashValue = facade.hashTransaction(transaction);

			// Assert:
			expect(hashValue).to.deep.equal(descriptor.transactionHash);
		});

		it(`can sign ${descriptor.name} transaction`, () => {
			// Arrange:
			const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
			const facade = new NemFacade('testnet');

			const transaction = descriptor.createTransaction();

			// Sanity:
			expect(transaction.signature).to.deep.equal(Signature.zero());

			// Act:
			const signature = facade.signTransaction(new NemFacade.KeyPair(privateKey), transaction);

			// Assert:
			expect(signature).to.deep.equal(descriptor.transactionSignature);
		});

		const assertCanVerifyTransaction = sign => {
			// Arrange:
			const privateKey = new PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC');
			const facade = new NemFacade('testnet');

			const transaction = descriptor.createTransaction();

			// Sanity:
			expect(transaction.signature).to.deep.equal(Signature.zero());

			// Act:
			const signature = sign(facade, new NemFacade.KeyPair(privateKey), transaction);
			const isVerified = facade.verifyTransaction(transaction, signature);

			// Assert:
			expect(isVerified).to.equal(true);
		};

		it(`can verify signed ${descriptor.name} transaction`, () => {
			assertCanVerifyTransaction((facade, keyPair, transaction) => facade.signTransaction(keyPair, transaction));
		});

		it(`can verify signed ${descriptor.name} transaction signing payload`, () => {
			assertCanVerifyTransaction((facade, keyPair, transaction) => {
				const signingPayload = facade.extractSigningPayload(transaction);
				return keyPair.sign(signingPayload);
			});
		});
	};

	const transferTestDescriptor = {
		name: 'normal',
		createTransaction: createRealTransfer,
		transactionHash: new Hash256('A7064DB890A4E7329AAB2AE7DCFA5EC76D7E374590C61EC85E03C698DF4EA79D'),
		transactionSignature: new Signature([
			'23A7B3433D16172E6C8659DB24233C5A8222C589098EA7A8FBBCB19691C67DB1',
			'3FB2AB7BB215265A3E3D74D32683516B03785BFEB2A2DE6DAC09F5E34A793706'
		].join(''))
	};

	addTransactionTests(transferTestDescriptor);

	// endregion

	// region multisig transaction

	const multisigTestDescriptor = {
		name: 'multisig',
		createTransaction: createRealMultisigTransaction,
		transactionHash: new Hash256('B585BC092CDDDCBA535FD6C0DE38F26EB44E6BA638A0BA6DFAD4BAA7E7AAE1B8'),
		transactionSignature: new Signature([
			'E324CCA57275D9752A684E6A089733803423647B8DDF5C1627FC23218CC84287',
			'EB7037AD4C6CB8CB37BBC9F5423FA73F431814A008400A756CFFE35F4533EB00'
		].join(''))
	};

	addTransactionTests(multisigTestDescriptor);

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
