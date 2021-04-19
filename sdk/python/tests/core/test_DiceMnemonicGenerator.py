import math
import random
import unittest
from binascii import unhexlify

from mnemonic import Mnemonic

from symbolchain.core.DiceMnemonicGenerator import DiceMnemonicGenerator


def mnemonic_to_seed(words):
    return Mnemonic('english').to_entropy(words)


class DiceMnemonicGeneratorTest(unittest.TestCase):
    # region add_roll

    def test_can_add_single_roll_in_range(self):
        # Arrange:
        for roll in [1, 2, 3, 4, 5, 6]:
            generator = DiceMnemonicGenerator()

            # Act:
            generator.add_roll(roll)

            # Assert:
            self.assertEqual([roll], generator.rolls)

    def test_can_add_multiple_rolls_in_range(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        # Act:
        for roll in [1, 6, 3, 5, 4, 2]:
            generator.add_roll(roll)

        # Assert:
        self.assertEqual([1, 6, 3, 5, 4, 2], generator.rolls)

    def test_cannot_add_roll_out_of_range(self):
        # Arrange:
        for roll in [-1, 0, 7, 8, 100]:
            generator = DiceMnemonicGenerator()

            # Act + Assert:
            with self.assertRaises(ValueError):
                generator.add_roll(roll)

    # endregion

    # region frequencies

    def test_frequencies_are_initially_zero(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        # Act:
        frequencies = generator.frequencies()

        # Assert:
        self.assertEqual([0] * 6, frequencies)

    def test_frequencies_are_calculated_correctly(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        rolls = [1] * 17 + [2] * 15 + [3] * 9 + [4] * 12 + [5] * 10 + [6] * 19
        random.shuffle(rolls)
        for roll in rolls:
            generator.add_roll(roll)

        # Act:
        frequencies = generator.frequencies()

        # Assert:
        self.assertEqual([17, 15, 9, 12, 10, 19], frequencies)

    def test_frequencies_are_calculated_correctly_custom_die(self):
        # Arrange:
        generator = DiceMnemonicGenerator(num_die_sides=8)

        rolls = [1] * 17 + [2] * 15 + [3] * 9 + [4] * 12 + [5] * 10 + [6] * 19 + [7] * 8 + [8] * 14
        random.shuffle(rolls)
        for roll in rolls:
            generator.add_roll(roll)

        # Act:
        frequencies = generator.frequencies()

        # Assert:
        self.assertEqual([17, 15, 9, 12, 10, 19, 8, 14], frequencies)

    # endregion

    # region to_mnemonic (shrink_wrap=*)

    def _assert_mnemonic_can_be_created_with_max_entropy(self, shrink_wrap):
        # Arrange:
        generator = DiceMnemonicGenerator()

        for roll in [1, 2] * 50:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic(shrink_wrap)
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(24, len(mnemonic.split()))
        self.assertEqual(256, entropy_bits)
        self.assertEqual(unhexlify('C6DDDB140A8D03482E696D47A911E89634049EFA7B2AF3297105FFEAE4EADB7C'), seed)

    def _assert_mnemonic_caps_entropy_at_256bits(self, shrink_wrap):
        # Arrange:
        generator = DiceMnemonicGenerator()

        for roll in [1, 2] * 100:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic(shrink_wrap)
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(24, len(mnemonic.split()))
        self.assertEqual(256, entropy_bits)
        self.assertEqual(unhexlify('DD979CB2857F8D1F48837A53A8DF9A2FA9FEF5D4CBC563C252EC9815267E7437'), seed)

    # endregion

    # region to_mnemonic (shrink_wrap=False)

    def test_mnemonic_returns_correct_mnemonic_for_known_rolls(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        for roll in [1, 2, 3, 4, 5, 6]:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic()
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(24, len(mnemonic.split()))
        self.assertEqual(-math.log2(1/6) * 6, entropy_bits)
        self.assertEqual(unhexlify('8D969EEF6ECAD3C29A3A629280E686CF0C3F5D5A86AFF3CA12020C923ADC6C92'), seed)

    def test_mnemonic_returns_correct_mnemonic_for_known_rolls_custom_die(self):
        # Arrange:
        generator = DiceMnemonicGenerator(num_die_sides=8)

        for roll in [1, 2, 3, 4, 5, 6, 7, 8]:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic()
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(24, len(mnemonic.split()))
        self.assertEqual(-math.log2(1/8) * 8, entropy_bits)
        self.assertEqual(unhexlify('EF797C8118F02DFB649607DD5D3F8C7623048C9C063D532CC95C5ED7A898A64F'), seed)

    def test_mnemonic_has_no_min_entropy(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic()
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(24, len(mnemonic.split()))
        self.assertEqual(0, entropy_bits)
        self.assertEqual(unhexlify('E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855'), seed)

    def test_mnemonic_can_be_created_with_max_entropy(self):
        self._assert_mnemonic_can_be_created_with_max_entropy(False)

    def test_mnemonic_caps_entropy_at_256bits(self):
        self._assert_mnemonic_caps_entropy_at_256bits(False)

    # endregion

    # region to_mnemonic (shrink_wrap=True)

    def test_mnemonic_shrink_wrap_returns_correct_mnemonic_for_known_rolls(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        for roll in [2, 3, 4, 2, 6, 5, 2, 6] * 6 + [3, 3]:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic(shrink_wrap=True)
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(12, len(mnemonic.split()))
        self.assertEqual(128, entropy_bits)
        self.assertEqual(unhexlify('49DBC749E6024695F3580B722E148341'), seed)

    def test_mnemonic_shrink_wrap_returns_correct_mnemonic_for_known_rolls_custom_die(self):
        # Arrange:
        generator = DiceMnemonicGenerator(num_die_sides=8)

        for roll in [2, 3, 4, 2, 6, 5, 2, 6] * 5 + [8, 7, 6]:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic(shrink_wrap=True)
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(12, len(mnemonic.split()))
        self.assertEqual(128, entropy_bits)
        self.assertEqual(unhexlify('3CCAD1E97E6770F667DD982F7D1408A4'), seed)

    def test_mnemonic_shrink_wrap_fails_with_entropy_less_than_128bits(self):
        # Arrange:
        for roll_count in [0, 25, 49]:
            generator = DiceMnemonicGenerator()

            for roll in [1] * roll_count:
                generator.add_roll(roll)

            with self.assertRaises(ValueError):
                generator.to_mnemonic(shrink_wrap=True)

    def test_mnemonic_shrink_wrap_can_be_created_with_min_entropy(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        for roll in [1, 2] * 25:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic(shrink_wrap=True)
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(12, len(mnemonic.split()))
        self.assertEqual(128, entropy_bits)
        self.assertEqual(unhexlify('11AB281EDD8D8F9789EC130530308357'), seed)

    def test_mnemonic_shrink_wrap_truncates_entropy_to_mnemonic_word_multiple(self):
        # Arrange:
        generator = DiceMnemonicGenerator()

        for roll in [1, 2] * 49 + [1]:
            generator.add_roll(roll)

        # Act:
        (mnemonic, entropy_bits) = generator.to_mnemonic(shrink_wrap=True)
        seed = mnemonic_to_seed(mnemonic)

        # Assert:
        self.assertEqual(21, len(mnemonic.split()))
        self.assertEqual(224, entropy_bits)
        self.assertEqual(unhexlify('FFC3D4A72768FB950D1D58BE65D509749416DE4A7C7C3C309382CBC7'), seed)

    def test_mnemonic_shrink_wrap_can_be_created_with_max_entropy(self):
        self._assert_mnemonic_can_be_created_with_max_entropy(True)

    def test_mnemonic_shrink_wrap_caps_entropy_at_256bits(self):
        self._assert_mnemonic_caps_entropy_at_256bits(True)

    # endregion
