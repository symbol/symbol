import unittest

from symbolchain import sc
from symbolchain.core.symbol.ExtendedTypeParsingRules import build_type_hints_map, extend_type_parsing_rules
from symbolchain.core.symbol.Network import Address


class TypeHintsMapTests(unittest.TestCase):
    def _assert_hints(self, transaction_type_hints, expected_type_hints):
        # Arrange:
        class MockTransaction:
            TYPE_HINTS = transaction_type_hints

        # Act:
        hints = build_type_hints_map(MockTransaction)

        # Assert:
        self.assertEqual(hints, expected_type_hints)

    def test_array_hints_are_copied(self):
        self._assert_hints({
            'alpha': 'array[alpha',
            'beta': 'array[beta]',
            'gamma': 'array[array[array[gamma',
            'delta': 'array('
        }, {
            'alpha': 'array[alpha',
            'beta': 'array[beta]',
            'gamma': 'array[array[array[gamma'
        })

    def test_enum_hints_are_copied_without_prefix(self):
        self._assert_hints({
            'alpha': 'enum:alpha',
            'beta': 'enum:beta12[]34',
            'gamma': 'enum:',
            'delta': 'enum'
        }, {
            'alpha': 'alpha',
            'beta': 'beta12[]34'
        })

    def test_pod_hints_are_copied_without_prefix(self):
        self._assert_hints({
            'alpha': 'pod:alpha',
            'beta': 'pod:beta12[]34',
            'gamma': 'pod:',
            'delta': 'pod'
        }, {
            'alpha': 'alpha',
            'beta': 'beta12[]34'
        })

    def test_struct_hints_are_copied(self):
        self._assert_hints({
            'alpha': 'struct:alpha',
            'beta': 'struct:beta12[]34',
            'gamma': 'struct:',
            'delta': 'struct'
        }, {
            'alpha': 'struct:alpha',
            'beta': 'struct:beta12[]34',
            'gamma': 'struct:'
        })

    def unknown_hints_are_not_copied(self):
        self._assert_hints({
            'alpha': 'class:alpha',
            'beta': 'union:beta12[]34'
        }, {})


class ExtendedTypeParsingRulesTests(unittest.TestCase):
    BASE_VALUES = [
        'Amount',
        'BlockDuration',
        'BlockFeeMultiplier',
        'Difficulty',
        'FinalizationEpoch',
        'FinalizationPoint',
        'Height',
        'Importance',
        'ImportanceHeight',
        'MosaicId',
        'MosaicNonce',
        'MosaicRestrictionKey',
        'NamespaceId',
        'ScopedMetadataKey',
        'Timestamp',
        'UnresolvedMosaicId'
    ]

    def _run_rules_parser(self, type_name, input_value, expected_outcome):
        # Arrange:
        rules = extend_type_parsing_rules(None, {})

        # Act:
        parsed = rules[type_name](input_value)

        # Assert:
        self.assertEqual(expected_outcome, parsed)

    def test_rules_contain_expected_hints(self):
        # Act:
        rules = extend_type_parsing_rules(None, {})

        # Assert:
        self.assertEqual(set(rules.keys()), set([
            'MosaicFlags',
            'AccountRestrictionFlags',
            'TransactionType',
            'AliasAction',
            'LinkAction',
            'LockHashAlgorithm',
            'MosaicRestrictionType',
            'MosaicSupplyChangeAction',
            'NamespaceRegistrationType',

            'struct:UnresolvedMosaic',

            'UnresolvedAddress',
            'Address',
            'Hash256',

            'array[UnresolvedMosaicId]',
            'array[TransactionType]',
            'array[UnresolvedAddress]',
            'array[UnresolvedMosaic]',
        ] + self.BASE_VALUES))

    # region PODs

    def _assert_base_value(self, input_value, expected_outcome):
        self._run_rules_parser('Amount', input_value, expected_outcome)

    def test_base_value_parser_forwards_to_ctor(self):
        self._assert_base_value(0x01234567_89ABCDEF, sc.Amount(0x01234567_89ABCDEF))

    # endregion

    # region flags

    def _assert_flags_parser(self, input_value, expected_outcome):
        self._run_rules_parser('MosaicFlags', input_value, expected_outcome)

    def test_flags_parser_can_handle_single_string_flag(self):
        self._assert_flags_parser('none', sc.MosaicFlags.NONE)

    def test_flags_parser_can_handle_multiple_string_flags(self):
        self._assert_flags_parser(
            'supply_mutable restrictable revokable',
            sc.MosaicFlags.SUPPLY_MUTABLE | sc.MosaicFlags.RESTRICTABLE | sc.MosaicFlags.REVOKABLE)

    def test_flags_parser_can_handle_ints(self):
        self._assert_flags_parser(9, sc.MosaicFlags.SUPPLY_MUTABLE | sc.MosaicFlags.REVOKABLE)

    def test_flags_parser_passes_non_parsed_values_as_is(self):
        value = sc.MosaicFlags.SUPPLY_MUTABLE | sc.MosaicFlags.RESTRICTABLE | sc.MosaicFlags.REVOKABLE
        self._assert_flags_parser(value, value)
        self._assert_flags_parser([1, 2, 3, 4], [1, 2, 3, 4])

    # endregion

    # region enum

    def _assert_enum_parser(self, input_value, expected_outcome):
        self._run_rules_parser('TransactionType', input_value, expected_outcome)

    def test_enum_parser_can_handle_string(self):
        self._assert_enum_parser('multisig_account_modification', sc.TransactionType.MULTISIG_ACCOUNT_MODIFICATION)

    def test_enum_parser_can_handle_ints(self):
        self._assert_enum_parser(16725, sc.TransactionType.MULTISIG_ACCOUNT_MODIFICATION)

    def test_enum_parser_passes_non_parsed_values_as_is(self):
        value = sc.TransactionType.MULTISIG_ACCOUNT_MODIFICATION
        self._assert_enum_parser(value, value)
        self._assert_enum_parser([1, 2, 3, 4], [1, 2, 3, 4])

    # endregion

    # region sdk type wrappers

    def _assert_sdk_wrapper(self, input_value, expected_outcome):
        self._run_rules_parser('UnresolvedAddress', input_value, expected_outcome)

    def test_sdk_wrapper_can_handle_proper_types(self):
        self._assert_sdk_wrapper('NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY', Address('NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY'))  # string
        self._assert_sdk_wrapper(b'hAAAABBBBBCCCCCDDDDDEEEE', Address('NBAUCQKBIJBEEQSCINBUGQ2DIRCEIRCEIVCUKRK'))  # raw bytes

    def test_sdk_wrapper_passes_non_parsed_values_as_is(self):
        value = Address('NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY')
        self._assert_sdk_wrapper(value, value)

    # endregion

    # region structs

    def test_struct_parser_forwards_to_transaction_descriptor_processor(self):
        # Arrange:
        # note: UnresolvedMosaic is currently only 'struct' handled, and it consists only of two base values
        type_breadcrumbs = []
        input_value = {'mosaic_id': 0x01234567_89ABCDEF, 'amount': 123_456_789_123_456_789}
        rules = extend_type_parsing_rules(lambda value: type_breadcrumbs.append(type(value)) or value, {})

        # Act:
        parsed = rules['struct:UnresolvedMosaic'](input_value)

        # Assert:
        self.assertEqual([sc.UnresolvedMosaicId, sc.Amount], type_breadcrumbs)
        self.assertEqual(sc.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed.mosaic_id)
        self.assertEqual(sc.Amount(123_456_789_123_456_789), parsed.amount)

    # endregion

    # region arrays

    def _assert_array_parser_transaction_type(self, input_value, expected_outcome):
        self._run_rules_parser('array[TransactionType]', input_value, expected_outcome)

    def test_array_parser_applies_non_array_parser(self):
        self._assert_array_parser_transaction_type(['hash_lock', 17230], [sc.TransactionType.HASH_LOCK, sc.TransactionType.MOSAIC_ALIAS])

    def _assert_array_parser_unresolved_mosaic(self, input_value, expected_outcome):
        self._run_rules_parser('array[UnresolvedMosaic]', input_value, expected_outcome)

    def test_array_parser_applies_struct_parser(self):
        # Arrange:
        rules = extend_type_parsing_rules(None, {})
        mosaics = [
            {'mosaic_id': 0x01234567_89ABCDEF, 'amount': 123_456_789_123_456_789},
            {'mosaic_id': 0x89ABCDEF_01234567, 'amount': 456_789_123_456_789_123}
        ]

        # Act:
        parsed = rules['array[UnresolvedMosaic]'](mosaics)

        # Assert:
        self.assertEqual(sc.UnresolvedMosaicId(0x01234567_89ABCDEF), parsed[0].mosaic_id)
        self.assertEqual(sc.Amount(123_456_789_123_456_789), parsed[0].amount)
        self.assertEqual(sc.UnresolvedMosaicId(0x89ABCDEF_01234567), parsed[1].mosaic_id)
        self.assertEqual(sc.Amount(456_789_123_456_789_123), parsed[1].amount)

    # endregion
