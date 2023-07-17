import BaseValue from '../src/BaseValue.js';
import ByteArray from '../src/ByteArray.js';
import { Hash256, PublicKey } from '../src/CryptoTypes.js';
import RuleBasedTransactionFactory from '../src/RuleBasedTransactionFactory.js';
import { expect } from 'chai';

describe('RuleBasedTransactionFactory', () => {
	// region Module

	class Module {
		static BaseValueAlias = BaseValue;

		static SigningPublicKey = PublicKey;

		// intentionally shadow Hash256 to emulate code produced by catbuffer generator, which shadows byte array types
		// eslint-disable-next-line no-shadow
		static Hash256 = class Hash256 extends ByteArray {
			constructor(hash256) {
				super(32, hash256);
			}
		};

		static MosaicFlags = class MosaicFlags {
			static NONE = new MosaicFlags(0);

			static SUPPLY_MUTABLE = new MosaicFlags(1);

			static TRANSFERABLE = new MosaicFlags(2);

			static RESTRICTABLE = new MosaicFlags(4);

			static REVOKABLE = new MosaicFlags(8);

			constructor(value) {
				this.value = value;
			}

			static or(...flagsArray) {
				return new MosaicFlags(flagsArray.map(flag => flag.value).reduce((x, y) => x | y));
			}
		};

		static NetworkType = class NetworkType {
			static MAINNET = new NetworkType(104);

			static TESTNET = new NetworkType(152);

			constructor(value) {
				this.value = value;
			}
		};

		static UnresolvedMosaicId = class UnresolvedMosaicId extends BaseValue {
			constructor(unresolvedMosaicId = 0n) {
				super(8, unresolvedMosaicId);
			}
		};

		static Amount = class Amount extends BaseValue {
			constructor(amount = 0n) {
				super(8, amount);
			}
		};

		static UnresolvedMosaic = class UnresolvedMosaic {
			static TYPE_HINTS = {
				mosaicId: 'pod:UnresolvedMosaicId',
				amount: 'pod:Amount'
			};

			constructor() {
				this.mosaicId = new Module.UnresolvedMosaicId();
				this.amount = new Module.Amount();
			}
		};

		static StructPlain = class StructPlain {
			static TYPE_HINTS = {};

			constructor() {
				this.mosaicId = 0;
				this.amount = 0;
			}
		};

		static StructArrayMember = class StructArrayParse {
			static TYPE_HINTS = {
				mosaicIds: 'array[UnresolvedMosaicId]'
			};

			constructor() {
				this.mosaicIds = [];
			}
		};

		static StructEnumMember = class StructEnumMember {
			static TYPE_HINTS = {
				network: 'enum:NetworkType'
			};

			constructor() {
				this.network = null;
			}
		};

		static StructStructMember = class StructStructMember {
			static TYPE_HINTS = {
				mosaic: 'struct:UnresolvedMosaic'
			};

			constructor() {
				this.mosaic = null;
			}
		};

		static StructHashMember = class StructHashMember {
			static TYPE_HINTS = {
				hash: 'Hash256'
			};

			constructor() {
				this.hash = null;
			}
		};
	}

	// endregion

	// region pod parser

	const requireRule = (factory, ruleName) => {
		const rule = factory.rules.get(ruleName);
		if (!rule)
			throw Error(`no rule with name ${ruleName}`);

		return rule;
	};

	describe('pod parser', () => {
		const assertPodParser = (inputValue, expectedValue, typingRules) => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module, undefined, typingRules);
			factory.addPodParser('SigningPublicKey', PublicKey);
			const rule = requireRule(factory, 'SigningPublicKey');

			// Act:
			const parsed = rule(inputValue);

			// Assert:
			expect(parsed).to.deep.equal(expectedValue);
		};

		it('can handle raw value', () => {
			assertPodParser(
				'364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28',
				new PublicKey('364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28'),
				undefined
			);
		});

		it('can handle typed value', () => {
			const publicKey = new PublicKey('364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28');
			assertPodParser(publicKey, publicKey, undefined);
		});

		it('uses type rule override when available', () => {
			// Arrange:
			const typeRuleOverrides = new Map();
			typeRuleOverrides.set(PublicKey, value => `pubkey ${value}`);

			// Act + Assert:
			assertPodParser(
				new PublicKey('364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28'),
				'pubkey 364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28',
				typeRuleOverrides
			);
		});
	});

	// endregion

	// region flags parser

	describe('flags parser', () => {
		const assertFlagsParser = (inputValue, expectedValue) => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addFlagsParser('MosaicFlags');
			const rule = requireRule(factory, 'MosaicFlags');

			// Act:
			const parsed = rule(inputValue);

			// Assert:
			expect(parsed).to.deep.equal(expectedValue);
		};

		it('can handle single string flag', () => {
			assertFlagsParser('restrictable', Module.MosaicFlags.RESTRICTABLE);
		});

		it('can handle multiple string flags', () => {
			assertFlagsParser(
				'supply_mutable restrictable revokable',
				Module.MosaicFlags.or(Module.MosaicFlags.SUPPLY_MUTABLE, Module.MosaicFlags.RESTRICTABLE, Module.MosaicFlags.REVOKABLE)
			);
		});

		it('fails if any string is unknown', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addFlagsParser('MosaicFlags');
			const rule = requireRule(factory, 'MosaicFlags');

			// Act + Assert:
			expect(() => { rule('supply_mutable foo revokable'); }).to.throw('unknown value foo for type MosaicFlags');
		});

		it('can handle ints', () => {
			assertFlagsParser(9, Module.MosaicFlags.or(Module.MosaicFlags.SUPPLY_MUTABLE, Module.MosaicFlags.REVOKABLE));
		});

		it('passes non parsed values as is', () => {
			const value = Module.MosaicFlags.or(
				Module.MosaicFlags.SUPPLY_MUTABLE,
				Module.MosaicFlags.RESTRICTABLE,
				Module.MosaicFlags.REVOKABLE
			);
			assertFlagsParser(value, value);
			assertFlagsParser(1.2, 1.2);
			assertFlagsParser([1, 2, 3, 4], [1, 2, 3, 4]);
		});
	});

	// endregion

	// region enum parser

	describe('enum parser', () => {
		const assertEnumParser = (inputValue, expectedValue) => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addEnumParser('NetworkType');
			const rule = requireRule(factory, 'NetworkType');

			// Act:
			const parsed = rule(inputValue);

			// Assert:
			expect(parsed).to.deep.equal(expectedValue);
		};

		it('can handle string', () => {
			assertEnumParser('testnet', Module.NetworkType.TESTNET);
		});

		it('fails if string is unknown', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addEnumParser('NetworkType');
			const rule = requireRule(factory, 'NetworkType');

			// Act + Assert:
			expect(() => { rule('Bitcoin'); }).to.throw('unknown value Bitcoin for type NetworkType');
		});

		it('can handle ints', () => {
			assertEnumParser(152, Module.NetworkType.TESTNET);
		});

		it('passes non parsed values as is', () => {
			const value = Module.NetworkType.TESTNET;
			assertEnumParser(value, value);
			assertEnumParser(1.2, 1.2);
			assertEnumParser([1, 2, 3, 4], [1, 2, 3, 4]);
		});
	});

	// endregion

	// region struct parser

	describe('struct parser', () => {
		it('can parse plain fields', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addStructParser('StructPlain');
			const rule = requireRule(factory, 'struct:StructPlain');

			// Act:
			const parsed = rule({
				mosaicId: 0x01234567_89ABCDEFn,
				amount: 123_456_789_123_456_789n
			});

			// Assert:
			expect(parsed.mosaicId).to.equal(0x01234567_89ABCDEFn);
			expect(parsed.amount).to.equal(123_456_789_123_456_789n);
		});

		it('can parse pod fields', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addPodParser('Amount', Module.Amount);
			factory.addStructParser('UnresolvedMosaic');
			const rule = requireRule(factory, 'struct:UnresolvedMosaic');

			// Act:
			const parsed = rule({
				mosaicId: 0x01234567_89ABCDEFn,
				amount: 123_456_789_123_456_789n
			});

			// Assert:
			expect(parsed.mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed.amount).to.deep.equal(new Module.Amount(123_456_789_123_456_789n));

			expect(parsed.mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
			expect(parsed.amount instanceof Module.Amount).to.equal(true);
		});

		it('can parse array fields', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addArrayParser('UnresolvedMosaicId');
			factory.addStructParser('StructArrayMember');
			const rule = requireRule(factory, 'struct:StructArrayMember');

			// Act:
			const parsed = rule({
				mosaicIds: [0x01234567_89ABCDEFn, 0x34567891_23456789n]
			});

			// Assert:
			expect(parsed.mosaicIds[0]).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed.mosaicIds[1]).to.deep.equal(new Module.UnresolvedMosaicId(0x34567891_23456789n));

			expect(parsed.mosaicIds[0] instanceof Module.UnresolvedMosaicId).to.equal(true);
			expect(parsed.mosaicIds[1] instanceof Module.UnresolvedMosaicId).to.equal(true);
		});

		it('can parse enum fields', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addEnumParser('NetworkType');
			factory.addStructParser('StructEnumMember');
			const rule = requireRule(factory, 'struct:StructEnumMember');

			// Act:
			const parsed = rule({
				network: 'testnet'
			});

			// Assert:
			expect(parsed.network).to.deep.equal(Module.NetworkType.TESTNET);
		});

		it('can parse struct fields', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addPodParser('Amount', Module.Amount);
			factory.addStructParser('UnresolvedMosaic');
			factory.addStructParser('StructStructMember');
			const rule = requireRule(factory, 'struct:StructStructMember');

			// Act:
			const parsed = rule({
				mosaic: {
					mosaicId: 0x01234567_89ABCDEFn,
					amount: 123_456_789_123_456_789n
				}
			});

			// Assert:
			expect(parsed.mosaic.mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed.mosaic.amount).to.deep.equal(new Module.Amount(123_456_789_123_456_789n));

			expect(parsed.mosaic.mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
			expect(parsed.mosaic.amount instanceof Module.Amount).to.equal(true);
		});

		it('can parse with type converter', () => {
			// Arrange: use custom type converter that unwraps amounts
			const factory = new RuleBasedTransactionFactory(Module, value => (value instanceof Module.Amount ? value.value : value));
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addPodParser('Amount', Module.Amount);
			factory.addStructParser('UnresolvedMosaic');
			const rule = requireRule(factory, 'struct:UnresolvedMosaic');

			// Act:
			const parsed = rule({
				mosaicId: 0x01234567_89ABCDEFn,
				amount: 123_456_789_123_456_789n
			});

			// Assert:
			expect(parsed.mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed.amount).to.equal(123_456_789_123_456_789n);

			expect(parsed.mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
		});

		it('can parse with type converter and autodetect byte arrays', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addPodParser('Hash256', Module.Hash256);
			factory.addStructParser('StructHashMember');
			const rule = requireRule(factory, 'struct:StructHashMember');

			// Act:
			const parsed = rule({
				hash: new Hash256('E9B3AEDE9A57C2B8C3D78DB9805D12AB0D983B63CE8F89D8DFE108D0FF08D23C')
			});

			// Assert:
			expect(parsed.hash).to.deep.equal(new Module.Hash256('E9B3AEDE9A57C2B8C3D78DB9805D12AB0D983B63CE8F89D8DFE108D0FF08D23C'));

			expect(parsed.hash instanceof Module.Hash256).to.equal(true);
		});
	});

	// endregion

	// region array parser

	describe('array parser', () => {
		it('cannot add with unknown element type', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);

			// Act + Assert:
			expect(() => factory.addArrayParser('NetworkType')).to.throw('element rule "NetworkType" is unknown');
		});

		it('can parse enum array', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addEnumParser('NetworkType');
			factory.addArrayParser('NetworkType');
			const rule = requireRule(factory, 'array[NetworkType]');

			// Act:
			const parsed = rule(['mainnet', 152]);

			// Assert:
			expect(parsed).to.deep.equal([Module.NetworkType.MAINNET, Module.NetworkType.TESTNET]);
		});

		it('can parse struct array', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addPodParser('Amount', Module.Amount);
			factory.addStructParser('UnresolvedMosaic');
			factory.addArrayParser('struct:UnresolvedMosaic');
			const rule = requireRule(factory, 'array[UnresolvedMosaic]');

			// Act:
			const parsed = rule([
				{ mosaicId: 0x01234567_89ABCDEFn, amount: 123_456_789_123_456_789n },
				{ mosaicId: 0x89ABCDEF_01234567n, amount: 456_789_123_456_789_123n }
			]);

			// Assert:
			expect(parsed[0].mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed[0].amount).to.deep.equal(new Module.Amount(123_456_789_123_456_789n));
			expect(parsed[1].mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x89ABCDEF_01234567n));
			expect(parsed[1].amount).to.deep.equal(new Module.Amount(456_789_123_456_789_123n));

			expect(parsed[0].mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
			expect(parsed[0].amount instanceof Module.Amount).to.equal(true);
			expect(parsed[1].mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
			expect(parsed[1].amount instanceof Module.Amount).to.equal(true);
		});
	});

	// endregion

	// region autodetect

	describe('autodetect', () => {
		it('adds pod rules', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);

			// Act:
			factory.autodetect();

			// Assert:
			expect(Array.from(factory.rules.keys())).to.deep.equal(['UnresolvedMosaicId', 'Amount']);
			expect(requireRule(factory, 'UnresolvedMosaicId')(123n)).to.deep.equal(new Module.UnresolvedMosaicId(123n));
			expect(requireRule(factory, 'Amount')(987n)).to.deep.equal(new Module.Amount(987n));
		});
	});

	// endregion

	// region create from factory

	describe('create from factory', () => {
		it('can create simple struct', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addStructParser('StructPlain');
			const entityFactory = entityType => (123 !== entityType ? undefined : new Module.StructPlain());

			// Act:
			const parsed = factory.createFromFactory(entityFactory, {
				type: 123,
				mosaicId: 0x01234567_89ABCDEFn,
				amount: 123_456_789_123_456_789n
			});

			// Assert:
			expect(parsed.mosaicId).to.equal(0x01234567_89ABCDEFn);
			expect(parsed.amount).to.equal(123_456_789_123_456_789n);
			expect(parsed.type).to.equal(undefined);
		});

		it('can create struct with nested rules', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addPodParser('Amount', Module.Amount);
			factory.addStructParser('UnresolvedMosaic');
			const entityFactory = entityType => (123 !== entityType ? undefined : new Module.UnresolvedMosaic());

			// Act:
			const parsed = factory.createFromFactory(entityFactory, {
				type: 123,
				mosaicId: 0x01234567_89ABCDEFn,
				amount: 123_456_789_123_456_789n
			});

			// Assert:
			expect(parsed.mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed.amount).to.deep.equal(new Module.Amount(123_456_789_123_456_789n));
			expect(parsed.type).to.equal(undefined);

			expect(parsed.mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
			expect(parsed.amount instanceof Module.Amount).to.equal(true);
		});

		it('can create struct with type converter', () => {
			// Arrange: use custom type converter that unwraps amounts
			const factory = new RuleBasedTransactionFactory(Module, value => (value instanceof Module.Amount ? value.value : value));
			factory.addPodParser('UnresolvedMosaicId', Module.UnresolvedMosaicId);
			factory.addPodParser('Amount', Module.Amount);
			factory.addStructParser('UnresolvedMosaic');
			const entityFactory = entityType => (123 !== entityType ? undefined : new Module.UnresolvedMosaic());

			// Act:
			const parsed = factory.createFromFactory(entityFactory, {
				type: 123,
				mosaicId: 0x01234567_89ABCDEFn,
				amount: 123_456_789_123_456_789n
			});

			// Assert:
			expect(parsed.mosaicId).to.deep.equal(new Module.UnresolvedMosaicId(0x01234567_89ABCDEFn));
			expect(parsed.amount).to.deep.equal(123_456_789_123_456_789n);
			expect(parsed.type).to.equal(undefined);

			expect(parsed.mosaicId instanceof Module.UnresolvedMosaicId).to.equal(true);
		});

		it('can create struct and auto encode strings', () => {
			// Arrange: use a plain struct but set string values
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addStructParser('StructPlain');
			const entityFactory = entityType => (123 !== entityType ? undefined : new Module.StructPlain());

			// Act:
			const parsed = factory.createFromFactory(entityFactory, {
				type: 123,
				mosaicId: '01234567_89ABCDEF',
				amount: '123_456_789_123_456_789'
			});

			// Assert: string values were encoded into utf8
			expect(parsed.mosaicId).to.deep.equal(new Uint8Array([
				0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x5F, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46
			]));
			expect(parsed.amount).to.deep.equal(new Uint8Array([
				0x31, 0x32, 0x33, 0x5F, 0x34, 0x35, 0x36, 0x5F, 0x37, 0x38, 0x39, 0x5F,
				0x31, 0x32, 0x33, 0x5F, 0x34, 0x35, 0x36, 0x5F, 0x37, 0x38, 0x39
			]));
			expect(parsed.type).to.equal(undefined);
		});

		it('cannot create struct when descriptor does not have type', () => {
			// Arrange:
			const factory = new RuleBasedTransactionFactory(Module);
			factory.addStructParser('StructPlain');
			const entityFactory = () => new Module.StructPlain();

			// Act:
			expect(() => {
				factory.createFromFactory(entityFactory, {
					mosaicId: 0x01234567_89ABCDEFn,
					amount: 123_456_789_123_456_789n
				});
			}).to.throw('transaction descriptor does not have attribute type');
		});
	});

	// endregion
});
