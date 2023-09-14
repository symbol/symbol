import unittest
from enum import Enum, Flag

from symbolchain.BaseValue import BaseValue
from symbolchain.ByteArray import ByteArray
from symbolchain.CryptoTypes import Hash256, PublicKey
from symbolchain.RuleBasedTransactionFactory import RuleBasedTransactionFactory

# region Module


class Module:
	EnumAlias = Enum
	FlagAlias = Flag
	BaseValueAlias = BaseValue

	SigningPublicKey = PublicKey

	class Hash256(ByteArray):
		def __init__(self, hash256):
			super().__init__(32, hash256, Module.Hash256)

	class MosaicFlags(Flag):
		NONE = 0
		SUPPLY_MUTABLE = 1
		TRANSFERABLE = 2
		RESTRICTABLE = 4
		REVOKABLE = 8

	class NetworkType(Enum):
		MAINNET = 104
		TESTNET = 152

	class UnresolvedMosaicId(BaseValue):
		def __init__(self, unresolved_mosaic_id=0):
			super().__init__(8, unresolved_mosaic_id, Module.UnresolvedMosaicId)

	class Amount(BaseValue):
		def __init__(self, amount=0):
			super().__init__(8, amount, Module.Amount)

	class UnresolvedMosaic:
		TYPE_HINTS = {
			'mosaic_id': 'pod:UnresolvedMosaicId',
			'amount': 'pod:Amount'
		}

		def __init__(self):
			self.mosaic_id = Module.UnresolvedMosaicId()
			self.amount = Module.Amount()

	class StructPlain:
		TYPE_HINTS = {}

		def __init__(self):
			self.mosaic_id = 0
			self.amount = 0

	class StructArrayMember:
		TYPE_HINTS = {
			'mosaic_ids': 'array[UnresolvedMosaicId]'
		}

		def __init__(self):
			self.mosaic_ids = []

	class StructEnumMember:
		TYPE_HINTS = {
			'network': 'enum:NetworkType'
		}

		def __init__(self):
			self.network = None

	class StructStructMember:
		TYPE_HINTS = {
			'mosaic': 'struct:UnresolvedMosaic'
		}

		def __init__(self):
			self.mosaic = None

	class StructHashMember:
		TYPE_HINTS = {
			'hash': 'Hash256'
		}

		def __init__(self):
			self.hash = None


# endregion

class RuleBasedTransactionFactoryTest(unittest.TestCase):
	# pylint: disable=too-many-public-methods

	# region pod parser

	def _assert_pod_parser(self, input_value, expected_value, typing_rules=None):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module, None, typing_rules)
		factory.add_pod_parser('SigningPublicKey', PublicKey)
		rule = factory.rules['SigningPublicKey']

		# Act:
		parsed = rule(input_value)

		# Assert:
		self.assertEqual(expected_value, parsed)

	def test_pod_parser_can_handle_raw_value(self):
		self._assert_pod_parser(
			'364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28',
			PublicKey('364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28'))

	def test_pod_parser_can_handle_typed_value(self):
		public_key = PublicKey('364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28')
		self._assert_pod_parser(public_key, public_key)

	def test_pod_parser_uses_type_rule_override_when_available(self):
		self._assert_pod_parser(
			PublicKey('364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28'),
			'pubkey 364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28',
			{PublicKey: lambda value: f'pubkey {value}'})

	# endregion

	# region flags parser

	def _assert_flags_parser(self, input_value, expected_value):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_flags_parser('MosaicFlags')
		rule = factory.rules['MosaicFlags']

		# Act:
		parsed = rule(input_value)

		# Assert:
		self.assertEqual(expected_value, parsed)

	def test_flags_parser_can_handle_none_string_flag(self):
		self._assert_flags_parser('none', Module.MosaicFlags.NONE)

	def test_flags_parser_can_handle_single_string_flag(self):
		self._assert_flags_parser('restrictable', Module.MosaicFlags.RESTRICTABLE)

	def test_flags_parser_can_handle_multiple_string_flags(self):
		self._assert_flags_parser(
			'supply_mutable restrictable revokable',
			Module.MosaicFlags.SUPPLY_MUTABLE | (Module.MosaicFlags.RESTRICTABLE | Module.MosaicFlags.REVOKABLE))

	def test_flags_parser_fails_if_any_string_is_unknown(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_flags_parser('MosaicFlags')
		rule = factory.rules['MosaicFlags']

		# Act + Assert;
		with self.assertRaises(ValueError):
			rule('supply_mutable foo revokable')

	def test_flags_parser_can_handle_ints(self):
		self._assert_flags_parser(9, Module.MosaicFlags.SUPPLY_MUTABLE | Module.MosaicFlags.REVOKABLE)

	def test_flags_parser_passes_non_parsed_values_as_is(self):
		value = Module.MosaicFlags.SUPPLY_MUTABLE | (Module.MosaicFlags.RESTRICTABLE | Module.MosaicFlags.REVOKABLE)
		self._assert_flags_parser(value, value)
		self._assert_flags_parser(1.2, 1.2)
		self._assert_flags_parser([1, 2, 3, 4], [1, 2, 3, 4])

	# endregion

	# region enum parser

	def _assert_enum_parser(self, input_value, expected_value):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_enum_parser('NetworkType')
		rule = factory.rules['NetworkType']

		# Act:
		parsed = rule(input_value)

		# Assert:
		self.assertEqual(expected_value, parsed)

	def test_enum_parser_can_handle_string(self):
		self._assert_enum_parser('testnet', Module.NetworkType.TESTNET)

	def test_enum_parser_fails_if_string_is_unknown(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_enum_parser('NetworkType')
		rule = factory.rules['NetworkType']

		# Act + Assert:
		with self.assertRaises(ValueError):
			rule('Bitcoin')

	def test_enum_parser_can_handle_ints(self):
		self._assert_enum_parser(152, Module.NetworkType.TESTNET)

	def test_enum_parser_passes_non_parsed_values_as_is(self):
		value = Module.NetworkType.TESTNET
		self._assert_enum_parser(value, value)
		self._assert_enum_parser(1.2, 1.2)
		self._assert_enum_parser([1, 2, 3, 4], [1, 2, 3, 4])

	# endregion

	# region struct parser

	def test_struct_parser_can_parse_plain_fields(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_struct_parser('StructPlain')
		rule = factory.rules['struct:StructPlain']

		# Act:
		parsed = rule({
			'mosaic_id': 0x01234567_89ABCDEF,
			'amount': 123_456_789_123_456_789
		})

		# Assert:
		self.assertEqual(0x01234567_89ABCDEF, parsed.mosaic_id)
		self.assertEqual(123_456_789_123_456_789, parsed.amount)

	def test_struct_parser_can_parse_pod_fields(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_pod_parser('Amount', Module.Amount)
		factory.add_struct_parser('UnresolvedMosaic')
		rule = factory.rules['struct:UnresolvedMosaic']

		# Act:
		parsed = rule({
			'mosaic_id': 0x01234567_89ABCDEF,
			'amount': 123_456_789_123_456_789
		})

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic_id)
		self.assertEqual(Module.Amount(123_456_789_123_456_789), parsed.amount)

	def test_struct_parser_can_parse_array_fields(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_array_parser('UnresolvedMosaicId')
		factory.add_struct_parser('StructArrayMember')
		rule = factory.rules['struct:StructArrayMember']

		# Act:
		parsed = rule({
			'mosaic_ids': [0x01234567_89ABCDEF, 0x34567891_23456789]
		})

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic_ids[0])
		self.assertEqual(Module.UnresolvedMosaicId(0x34567891_23456789), parsed.mosaic_ids[1])

	def test_struct_parser_can_parse_enum_fields(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_enum_parser('NetworkType')
		factory.add_struct_parser('StructEnumMember')
		rule = factory.rules['struct:StructEnumMember']

		# Act:
		parsed = rule({
			'network': 'testnet'
		})

		# Assert:
		self.assertEqual(Module.NetworkType.TESTNET, parsed.network)

	def test_struct_parser_can_parse_struct_fields(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_pod_parser('Amount', Module.Amount)
		factory.add_struct_parser('UnresolvedMosaic')
		factory.add_struct_parser('StructStructMember')
		rule = factory.rules['struct:StructStructMember']

		# Act:
		parsed = rule({
			'mosaic': {
				'mosaic_id': 0x01234567_89ABCDEF,
				'amount': 123_456_789_123_456_789
			}
		})

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic.mosaic_id)
		self.assertEqual(Module.Amount(123_456_789_123_456_789), parsed.mosaic.amount)

	def test_struct_parser_can_parse_with_type_converter(self):
		# Arrange: use custom type converter that unwraps amounts
		factory = RuleBasedTransactionFactory(Module, lambda value: value.value if isinstance(value, Module.Amount) else value)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_pod_parser('Amount', Module.Amount)
		factory.add_struct_parser('UnresolvedMosaic')
		rule = factory.rules['struct:UnresolvedMosaic']

		# Act:
		parsed = rule({
			'mosaic_id': 0x01234567_89ABCDEF,
			'amount': 123_456_789_123_456_789
		})

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic_id)
		self.assertEqual(123_456_789_123_456_789, parsed.amount)

	def test_struct_parser_can_parse_with_type_converter_and_autodetect_byte_arrays(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_pod_parser('Hash256', Module.Hash256)
		factory.add_struct_parser('StructHashMember')
		rule = factory.rules['struct:StructHashMember']

		# Act:
		parsed = rule({
			'hash': Hash256('E9B3AEDE9A57C2B8C3D78DB9805D12AB0D983B63CE8F89D8DFE108D0FF08D23C')
		})

		# Assert:
		self.assertEqual(Module.Hash256('E9B3AEDE9A57C2B8C3D78DB9805D12AB0D983B63CE8F89D8DFE108D0FF08D23C'), parsed.hash)

	# endregion

	# region array parser

	def test_array_parser_can_parse_enum_array(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_enum_parser('NetworkType')
		factory.add_array_parser('NetworkType')
		rule = factory.rules['array[NetworkType]']

		# Act:
		parsed = rule(['mainnet', 152])

		# Assert:
		self.assertEqual([Module.NetworkType.MAINNET, Module.NetworkType.TESTNET], parsed)

	def test_array_parser_can_parse_struct_array(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_pod_parser('Amount', Module.Amount)
		factory.add_struct_parser('UnresolvedMosaic')
		factory.add_array_parser('struct:UnresolvedMosaic')
		rule = factory.rules['array[UnresolvedMosaic]']

		# Act:
		parsed = rule([
			{'mosaic_id': 0x01234567_89ABCDEF, 'amount': 123_456_789_123_456_789},
			{'mosaic_id': 0x89ABCDEF_01234567, 'amount': 456_789_123_456_789_123}
		])

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed[0].mosaic_id)
		self.assertEqual(Module.Amount(123_456_789_123_456_789), parsed[0].amount)
		self.assertEqual(Module.UnresolvedMosaicId(0x89ABCDEF_01234567), parsed[1].mosaic_id)
		self.assertEqual(Module.Amount(456_789_123_456_789_123), parsed[1].amount)

	# endregion

	# region autodetect

	def test_autodetect_adds_pod_and_enum_rules(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)

		# Act:
		factory.autodetect()

		# Assert:
		self.assertEqual(set(['MosaicFlags', 'NetworkType', 'UnresolvedMosaicId', 'Amount']), set(factory.rules.keys()))
		self.assertEqual(
			Module.MosaicFlags.REVOKABLE | Module.MosaicFlags.TRANSFERABLE,
			factory.rules['MosaicFlags']('revokable transferable'))
		self.assertEqual(Module.NetworkType.TESTNET, factory.rules['NetworkType']('testnet'))
		self.assertEqual(Module.UnresolvedMosaicId(123), factory.rules['UnresolvedMosaicId'](123))
		self.assertEqual(Module.Amount(987), factory.rules['Amount'](987))

	# endregion

	# region create_from_factory

	def test_can_create_simple_struct_from_factory(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_struct_parser('StructPlain')

		def entity_factory(entity_type):
			return None if 123 != entity_type else Module.StructPlain()

		# Act:
		parsed = factory.create_from_factory(entity_factory, {
			'type': 123,
			'mosaic_id': 0x01234567_89ABCDEF,
			'amount': 123_456_789_123_456_789
		})

		# Assert:
		self.assertEqual(0x01234567_89ABCDEF, parsed.mosaic_id)
		self.assertEqual(123_456_789_123_456_789, parsed.amount)
		self.assertFalse(hasattr(parsed, 'type'))

	def test_can_create_struct_from_factory_with_nested_rules(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_pod_parser('Amount', Module.Amount)
		factory.add_struct_parser('UnresolvedMosaic')

		def entity_factory(entity_type):
			return None if 123 != entity_type else Module.UnresolvedMosaic()

		# Act:
		parsed = factory.create_from_factory(entity_factory, {
			'type': 123,
			'mosaic_id': 0x01234567_89ABCDEF,
			'amount': 123_456_789_123_456_789
		})

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic_id)
		self.assertEqual(Module.Amount(123_456_789_123_456_789), parsed.amount)
		self.assertFalse(hasattr(parsed, 'type'))

	def test_can_create_struct_from_factory_with_type_converter(self):
		# Arrange: use custom type converter that unwraps amounts
		factory = RuleBasedTransactionFactory(Module, lambda value: value.value if isinstance(value, Module.Amount) else value)
		factory.add_pod_parser('UnresolvedMosaicId', Module.UnresolvedMosaicId)
		factory.add_pod_parser('Amount', Module.Amount)
		factory.add_struct_parser('UnresolvedMosaic')

		def entity_factory(entity_type):
			return None if 123 != entity_type else Module.UnresolvedMosaic()

		# Act:
		parsed = factory.create_from_factory(entity_factory, {
			'type': 123,
			'mosaic_id': 0x01234567_89ABCDEF,
			'amount': 123_456_789_123_456_789
		})

		# Assert:
		self.assertEqual(Module.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic_id)
		self.assertEqual(123_456_789_123_456_789, parsed.amount)
		self.assertFalse(hasattr(parsed, 'type'))

	def test_can_create_struct_from_factory_and_auto_encode_strings(self):
		# Arrange: use a plain struct but set string values
		factory = RuleBasedTransactionFactory(Module)
		factory.add_struct_parser('StructPlain')

		def entity_factory(entity_type):
			return None if 123 != entity_type else Module.StructPlain()

		# Act:
		parsed = factory.create_from_factory(entity_factory, {
			'type': 123,
			'mosaic_id': '01234567_89ABCDEF',
			'amount': '123_456_789_123_456_789'
		})

		# Assert: string values were encoded into utf8
		self.assertEqual(b'01234567_89ABCDEF', parsed.mosaic_id)
		self.assertEqual(b'123_456_789_123_456_789', parsed.amount)
		self.assertFalse(hasattr(parsed, 'type'))

	def test_cannot_create_struct_from_factory_when_descriptor_does_not_have_type(self):
		# Arrange:
		factory = RuleBasedTransactionFactory(Module)
		factory.add_struct_parser('StructPlain')

		def entity_factory(_):
			return Module.StructPlain()

		# Act:
		with self.assertRaises(ValueError):
			factory.create_from_factory(entity_factory, {
				'mosaic_id': 0x01234567_89ABCDEF,
				'amount': 123_456_789_123_456_789
			})

	# endregion
