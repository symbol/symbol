import unittest

from symbolchain.core.sym.IdGenerator import (generate_mosaic_alias_id, generate_mosaic_id, generate_namespace_id, generate_namespace_path,
                                              is_valid_namespace_name)
from symbolchain.core.sym.Network import Address

from ...test.NemTestUtils import NemTestUtils

TEST_VECTORS = {
    'uppercase': ['CAT.token', 'CAT.TOKEN', 'cat.TOKEN', 'cAt.ToKeN', 'CaT.tOkEn'],
    'improper_part': ['alpha.bet@.zeta', 'a!pha.beta.zeta', 'alpha.beta.ze^a'],
    'improper_qualified': ['.', '..', '...', '.a', 'b.', 'a..b', '.a.b', 'b.a.'],
}


class IdGeneratorTest(unittest.TestCase):
    # pylint: disable=too-many-public-methods

    # region generate_mosaic_id

    def test_generate_mosaic_id_generates_correct_id(self):
        # Act:
        mosaic_id = generate_mosaic_id(Address('TATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA37JGO5Q'), 812613930)

        # Assert:
        self.assertEqual(0x570FB3ED9379624C, mosaic_id)

    def test_generate_mosaic_id_different_addresses_produce_different_ids(self):
        # Arrange:
        address1 = Address(NemTestUtils.randcryptotype(Address))
        address2 = Address(NemTestUtils.randcryptotype(Address))

        # Act:
        mosaic_id1 = generate_mosaic_id(address1, 812613930)
        mosaic_id2 = generate_mosaic_id(address2, 812613930)

        # Assert:
        self.assertNotEqual(mosaic_id1, mosaic_id2)

    def test_generate_mosaic_id_different_nonces_produce_different_ids(self):
        # Arrange:
        address = Address(NemTestUtils.randcryptotype(Address))

        # Act:
        mosaic_id1 = generate_mosaic_id(address, 812613930)
        mosaic_id2 = generate_mosaic_id(address, 812613931)

        # Assert:
        self.assertNotEqual(mosaic_id1, mosaic_id2)

    def test_generate_mosaic_id_has_high_bit_cleared(self):
        # Arrange:
        for _ in range(0, 1000):
            address = Address(NemTestUtils.randcryptotype(Address))

            # Act:
            mosaic_id = generate_mosaic_id(address, 812613930)

            # Assert:
            self.assertEqual(0, mosaic_id >> 63)

    # endregion

    # region generate_namespace_id

    def test_generate_namespace_id_generates_correct_root_id(self):
        # Act:
        namespace_id = generate_namespace_id('symbol')

        # Assert:
        self.assertEqual(0xA95F1F8A96159516, namespace_id)

    def test_generate_namespace_id_generates_correct_child_id(self):
        # Act:
        namespace_id = generate_namespace_id('xym', 0xA95F1F8A96159516)

        # Assert:
        self.assertEqual(0xE74B99BA41F4AFEE, namespace_id)

    def test_generate_namespace_id_different_names_produce_different_ids(self):
        # Act:
        namespace_id1 = generate_namespace_id('symbol')
        namespace_id2 = generate_namespace_id('Symbol')

        # Assert:
        self.assertNotEqual(namespace_id1, namespace_id2)

    def test_generate_namespace_id_different_parents_produce_different_ids(self):
        # Act:
        namespace_id1 = generate_namespace_id('symbol', 0xA95F1F8A96159516)
        namespace_id2 = generate_namespace_id('symbol', 0xA95F1F8A96159517)

        # Assert:
        self.assertNotEqual(namespace_id1, namespace_id2)

    def test_generate_namespace_id_has_high_bit_set(self):
        # Arrange:
        for i in range(0, 1000):
            # Act:
            namespace_id = generate_namespace_id('symbol', i)

            # Assert:
            self.assertEqual(1, namespace_id >> 63)

    # endregion

    # region generate_mosaic_alias_id

    def test_generate_mosaic_alias_id_generates_correct_id(self):
        # Act:
        mosaic_id = generate_mosaic_alias_id('cat.token')

        # Assert:
        self.assertEqual(0xA029E100621B2E33, mosaic_id)

    def test_generate_mosaic_alias_id_supports_multilevel_mosaics(self):
        # Act:
        mosaic_id = generate_mosaic_alias_id('foo.bar.baz.xyz')

        # Assert:
        namespace_id = generate_namespace_id('baz', generate_namespace_id('bar', generate_namespace_id('foo')))
        expected_mosaic_id = generate_namespace_id('xyz', namespace_id)
        self.assertEqual(expected_mosaic_id, mosaic_id)

    def _assert_rejected_by_generate_mosaic_alias_id(self, names):
        for name in names:
            with self.assertRaises(ValueError):
                generate_mosaic_alias_id(name)

    def test_generate_mosaic_alias_id_rejects_uppercase_characters(self):
        self._assert_rejected_by_generate_mosaic_alias_id(TEST_VECTORS['uppercase'])

    def test_generate_mosaic_alias_id_rejects_improper_part_names(self):
        self._assert_rejected_by_generate_mosaic_alias_id(TEST_VECTORS['improper_part'])

    def test_generate_mosaic_alias_id_rejects_improper_qualified_names(self):
        self._assert_rejected_by_generate_mosaic_alias_id(TEST_VECTORS['improper_qualified'])

    def test_generate_mosaic_alias_id_rejects_empty_string(self):
        self._assert_rejected_by_generate_mosaic_alias_id([''])

    # endregion

    # region generate_namespace_path

    def test_generate_namespace_path_generates_correct_root_id(self):
        # Act:
        path = generate_namespace_path('cat')

        # Assert:
        self.assertEqual([0xB1497F5FBA651B4F], path)

    def test_generate_namespace_path_generates_correct_child_id(self):
        # Act:
        path = generate_namespace_path('cat.token')

        # Assert:
        self.assertEqual([0xB1497F5FBA651B4F, 0xA029E100621B2E33], path)

    def test_generate_namespace_path_supports_multilevel_namespaces(self):
        # Act:
        path = generate_namespace_path('foo.bar.baz.xyz')

        # Assert:
        expected_path = [generate_namespace_id('foo')]
        for name in ['bar', 'baz', 'xyz']:
            expected_path.append(generate_namespace_id(name, expected_path[-1]))

        self.assertEqual(expected_path, path)

    def _assert_rejected_by_generate_namespace_path(self, names):
        for name in names:
            with self.assertRaises(ValueError):
                generate_namespace_path(name)

    def test_generate_namespace_path_rejects_uppercase_characters(self):
        self._assert_rejected_by_generate_namespace_path(TEST_VECTORS['uppercase'])

    def test_generate_namespace_path_rejects_improper_part_names(self):
        self._assert_rejected_by_generate_namespace_path(TEST_VECTORS['improper_part'])

    def test_generate_namespace_path_rejects_improper_qualified_names(self):
        self._assert_rejected_by_generate_namespace_path(TEST_VECTORS['improper_qualified'])

    def test_generate_namespace_path_rejects_empty_string(self):
        self._assert_rejected_by_generate_namespace_path([''])

    # endregion

    # region is_valid_namespace_name

    def test_is_valid_namespace_name_returns_true_when_all_characters_are_alphanumeric(self):
        for name in ['a', 'be', 'cat', 'doom', '09az09', 'az09az']:
            self.assertTrue(is_valid_namespace_name(name))

    def test_is_valid_namespace_name_returns_true_when_name_contains_separator(self):
        for name in ['al-ce', 'al_ce', 'alice-', 'alice_']:
            self.assertTrue(is_valid_namespace_name(name))

    def test_is_valid_namespace_name_returns_false_when_name_starts_with_separator(self):
        for name in ['-alice', '_alice']:
            self.assertFalse(is_valid_namespace_name(name))

    def test_is_valid_namespace_name_returns_false_when_any_character_is_invalid(self):
        for name in ['al.ce', 'alIce', 'al ce', 'al@ce', 'al#ce']:
            self.assertFalse(is_valid_namespace_name(name))

    # endregion
