import random
import unittest

from symbolchain.core.BaseValue import BaseValue

from ..test.ComparisonTestUtils import ComparisonTestDescriptor, ComparisonTestUtils, EqualityTestDescriptor

WORD_WIDTH = 8

DEFAULT_VALUE = 0x456789AB_CDEF0123

HIBIT_SET_VALUE = 0xABCDEF01_23456789
HIBIT_UNSET_VALUE = DEFAULT_VALUE

# FE instead of FF, because tests use "+1", "-1" values
ORDER_TESTS_MAX_UNSIGNED = 0xFFFFFFFF_FFFFFFFE


class FakeValue:
    def __init__(self, value):
        self.value = value


class BaseValueTest(ComparisonTestUtils, unittest.TestCase):
    # pylint: disable=too-many-public-methods

    DESCRIPTOR_UNSIGNED = ComparisonTestDescriptor(
        lambda value: BaseValue(WORD_WIDTH, value, None, False),
        lambda value: BaseValue(WORD_WIDTH, value, str, False),
        FakeValue,
    )

    DESCRIPTOR_SIGNED = ComparisonTestDescriptor(
        lambda value: BaseValue(WORD_WIDTH, value, None, True),
        lambda value: BaseValue(WORD_WIDTH, value, str, True),
        FakeValue,
    )

    def test_can_create_base_value(self):
        # Act:
        val = BaseValue(WORD_WIDTH, DEFAULT_VALUE)

        # Assert:
        self.assertEqual(DEFAULT_VALUE, val.value)

    def test_width_is_capping(self):
        # Act:
        val = BaseValue(4, DEFAULT_VALUE)

        # Assert:
        self.assertNotEqual(DEFAULT_VALUE, val.value)
        self.assertEqual(0xCDEF0123, val.value)

    def test_unsigned_base_value_caps_negative_values(self):
        # Act + Assert:
        self.assertEqual(0xFFFF, BaseValue(2, -1).value)
        self.assertEqual(0xFFFB, BaseValue(2, -5).value)
        self.assertEqual(0x8001, BaseValue(2, -32767).value)
        self.assertEqual(0x8000, BaseValue(2, -32768).value)

        # - wraps due to "and" operator applied
        self.assertEqual(0x7FFF, BaseValue(2, -32769).value)
        self.assertEqual(0x7FFF, BaseValue(2, 32767).value)
        self.assertEqual(0x7FF4, BaseValue(2, -32780).value)

    def test_signed_base_value_caps_values(self):
        # Act + Assert: positive wrap around to negative
        self.assertEqual(126, BaseValue(1, 126, None, True).value)
        self.assertEqual(127, BaseValue(1, 127, None, True).value)
        self.assertEqual(-128, BaseValue(1, 128, None, True).value)
        self.assertEqual(-127, BaseValue(1, 129, None, True).value)

        self.assertEqual(-127, BaseValue(1, 129 + 1024, None, True).value)

        # - negative wrap around to positive
        self.assertEqual(-127, BaseValue(1, -127, None, True).value)
        self.assertEqual(-128, BaseValue(1, -128, None, True).value)
        self.assertEqual(127, BaseValue(1, -129, None, True).value)
        self.assertEqual(126, BaseValue(1, -130, None, True).value)

        self.assertEqual(126, BaseValue(1, -130 - 1024, None, True).value)

    def _equality_and_inequality_are_supported(self, descriptor):
        descriptor = EqualityTestDescriptor(
            descriptor.untagged,
            descriptor.tagged,
            lambda: random.randint(0, 2**64 - 1),
            FakeValue,
        )
        self.equality_is_supported(descriptor, DEFAULT_VALUE)
        self.inequality_is_supported(descriptor, DEFAULT_VALUE)

    def test_equality_is_supported_different_signedness(self):
        self.assertNotEqual(
            self.DESCRIPTOR_UNSIGNED.untagged(DEFAULT_VALUE),
            self.DESCRIPTOR_SIGNED.untagged(DEFAULT_VALUE))

        self.assertNotEqual(
            self.DESCRIPTOR_UNSIGNED.tagged(DEFAULT_VALUE),
            self.DESCRIPTOR_SIGNED.tagged(DEFAULT_VALUE))

    # region unsigned comparison tests

    def test_equality_and_inequality_are_supported_unsigned(self):
        self._equality_and_inequality_are_supported(self.DESCRIPTOR_UNSIGNED)

    def test_less_than_is_supported_unsigned(self):
        self.less_than_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
        self.less_than_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

    def test_less_than_or_equal_is_supported_unsigned(self):
        self.less_than_or_equal_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
        self.less_than_or_equal_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

    def test_greater_than_is_supported_unsigned(self):
        self.greater_than_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
        self.greater_than_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

    def test_greater_than_or_equal_is_supported_unsigned(self):
        self.greater_than_or_equal_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
        self.greater_than_or_equal_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

    # endregion

    # region signed comparison tests

    def test_equality_and_inequality_are_supported_signed(self):
        self._equality_and_inequality_are_supported(self.DESCRIPTOR_SIGNED)

    def test_less_than_is_supported_signed(self):
        self.less_than_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
        self.less_than_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
        self.assertLess(self.DESCRIPTOR_SIGNED.untagged(-3), self.DESCRIPTOR_SIGNED.untagged(5))

    def test_less_than_or_equal_is_supported_signed(self):
        self.less_than_or_equal_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
        self.less_than_or_equal_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
        self.assertLessEqual(self.DESCRIPTOR_SIGNED.untagged(-3), self.DESCRIPTOR_SIGNED.untagged(5))

    def test_greater_than_is_supported_signed(self):
        self.greater_than_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
        self.greater_than_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
        self.assertGreater(self.DESCRIPTOR_SIGNED.untagged(5), self.DESCRIPTOR_SIGNED.untagged(-3))

    def test_greater_than_or_equal_is_supported_signed(self):
        self.greater_than_or_equal_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
        self.greater_than_or_equal_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
        self.assertGreaterEqual(self.DESCRIPTOR_SIGNED.untagged(5), self.DESCRIPTOR_SIGNED.untagged(-3))

    # endregion

    def test_hash_is_supported(self):
        # Arrange:
        unsigned = self.DESCRIPTOR_UNSIGNED
        signed = self.DESCRIPTOR_SIGNED
        val_hash = hash(unsigned.untagged(DEFAULT_VALUE))

        # Act + Assert:
        self.assertEqual(val_hash, hash(unsigned.untagged(DEFAULT_VALUE)))

        self.assertNotEqual(hash(unsigned.untagged(DEFAULT_VALUE)), hash(unsigned.tagged(DEFAULT_VALUE)))

        self.assertNotEqual(hash(unsigned.untagged(HIBIT_SET_VALUE)), hash(signed.untagged(HIBIT_SET_VALUE)))
        self.assertNotEqual(hash(unsigned.untagged(HIBIT_UNSET_VALUE)), hash(signed.untagged(HIBIT_UNSET_VALUE)))

        self.assertNotEqual(val_hash, hash(unsigned.untagged(DEFAULT_VALUE - 1)))

        self.assertNotEqual(val_hash, None)

    def test_string_is_supported(self):
        self.assertEqual('0x456789ABCDEF0123', str(BaseValue(WORD_WIDTH, DEFAULT_VALUE)))
        self.assertEqual('0xABCDEF0123456789', str(BaseValue(WORD_WIDTH, HIBIT_SET_VALUE)))
        self.assertEqual('-0x543210FEDCBA9877', str(BaseValue(WORD_WIDTH, HIBIT_SET_VALUE, None, True)))

    def test_string_is_affected_by_width(self):
        self.assertEqual('0x000FEDCBA9876543', str(BaseValue(WORD_WIDTH, 0xFEDCBA9876543)))
        self.assertEqual('0x00000123', str(BaseValue(4, 0x123)))
        self.assertEqual('0x00000000', str(BaseValue(4, 0)))
        self.assertEqual('0x00', str(BaseValue(1, 0)))
