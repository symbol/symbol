import random
import unittest

from symbolchain.BaseValue import BaseValue

from .test.ComparisonTestUtils import ComparisonTestDescriptor, ComparisonTestUtils, EqualityTestDescriptor

WORD_WIDTH = 8

DEFAULT_VALUE = 0x456789AB_CDEF0123

HIBIT_SET_VALUE = 0xABCDEF01_23456789
HIBIT_SET_VALUE_SIGNED = -0x543210FEDCBA9877
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
		value = BaseValue(WORD_WIDTH, DEFAULT_VALUE)

		# Assert:
		self.assertEqual(WORD_WIDTH, value.size)
		self.assertEqual(DEFAULT_VALUE, value.value)

	def test_cannot_create_unsigned_with_values_outside_range(self):
		# Arrange:
		test_cases = [
			(1, -1), (1, 0x100),
			(2, -1), (2, 0x1_0000),
			(4, -1), (4, 0x1_0000_0000),
			(8, -1), (8, 0x1_0000_0000_0000_0000)
		]

		for (size, raw_value) in test_cases:
			# Act + Assert:
			with self.assertRaises(ValueError):
				BaseValue(size, raw_value, None, False)

	def test_can_create_unsigned_with_values_inside_range(self):
		# Arrange:
		test_cases = [
			(1, 0), (1, 0x24), (1, 0xFF),
			(2, 0), (2, 0x243F), (2, 0xFFFF),
			(4, 0), (4, 0x243F_6A88), (4, 0xFFFF_FFFF),
			(8, 0), (8, 0x243F_6A88_85A3_08D3), (8, 0xFFFF_FFFF_FFFF_FFFF)
		]

		for (size, raw_value) in test_cases:
			# Act:
			value = BaseValue(size, raw_value, None, False)

			# Assert:
			self.assertEqual(size, value.size)
			self.assertEqual(raw_value, value.value)

	def test_cannot_create_signed_with_values_outside_range(self):
		# Arrange:
		test_cases = [
			(1, -0x81), (1, 0x80),
			(2, -0x8001), (2, 0x8000),
			(4, -0x8000_0001), (4, 0x8000_0000),
			(8, -0x8000_0000_0000_0001), (8, 0x8000_0000_0000_0000)
		]

		for (size, raw_value) in test_cases:
			# Act + Assert:
			with self.assertRaises(ValueError):
				BaseValue(size, raw_value, None, True)

	def test_can_create_signed_with_values_inside_range(self):
		# Arrange:
		test_cases = [
			(1, -0x80), (1, 0x24), (1, 0x7F),
			(2, -0x8000), (2, 0x243F), (2, 0x7FFF),
			(4, -0x8000_0000), (4, 0x243F_6A88), (4, 0x7FFF_FFFF),
			(8, -0x8000_0000_0000_0000), (8, 0x243F_6A88_85A3_08D3), (8, 0x7FFF_FFFF_FFFF_FFFF)
		]

		for (size, raw_value) in test_cases:
			# Act:
			value = BaseValue(size, raw_value, None, True)

			# Assert:
			self.assertEqual(size, value.size)
			self.assertEqual(raw_value, value.value)

	def _equality_and_inequality_are_supported(self, descriptor, is_signed):
		descriptor = EqualityTestDescriptor(
			descriptor.untagged,
			descriptor.tagged,
			lambda: random.randint(0, 2 ** (32 if is_signed else 64) - 1),
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
		self._equality_and_inequality_are_supported(self.DESCRIPTOR_UNSIGNED, False)

	def test_less_than_is_supported_unsigned(self):
		self.less_than_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
		self.less_than_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

	def test_less_than_equal_is_supported_unsigned(self):
		self.less_than_equal_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
		self.less_than_equal_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

	def test_greater_than_is_supported_unsigned(self):
		self.greater_than_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
		self.greater_than_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

	def test_greater_than_equal_is_supported_unsigned(self):
		self.greater_than_equal_is_supported(self.DESCRIPTOR_UNSIGNED, DEFAULT_VALUE)
		self.greater_than_equal_is_supported(self.DESCRIPTOR_UNSIGNED, ORDER_TESTS_MAX_UNSIGNED)

	# endregion

	# region signed comparison tests

	def test_equality_and_inequality_are_supported_signed(self):
		self._equality_and_inequality_are_supported(self.DESCRIPTOR_SIGNED, True)

	def test_less_than_is_supported_signed(self):
		self.less_than_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
		self.less_than_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
		self.assertLess(self.DESCRIPTOR_SIGNED.untagged(-3), self.DESCRIPTOR_SIGNED.untagged(5))

	def test_less_than_equal_is_supported_signed(self):
		self.less_than_equal_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
		self.less_than_equal_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
		self.assertLessEqual(self.DESCRIPTOR_SIGNED.untagged(-3), self.DESCRIPTOR_SIGNED.untagged(5))

	def test_greater_than_is_supported_signed(self):
		self.greater_than_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
		self.greater_than_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
		self.assertGreater(self.DESCRIPTOR_SIGNED.untagged(5), self.DESCRIPTOR_SIGNED.untagged(-3))

	def test_greater_than_equal_is_supported_signed(self):
		self.greater_than_equal_is_supported(self.DESCRIPTOR_SIGNED, DEFAULT_VALUE)
		self.greater_than_equal_is_supported(self.DESCRIPTOR_SIGNED, -DEFAULT_VALUE)
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

		self.assertNotEqual(hash(unsigned.untagged(HIBIT_SET_VALUE)), hash(signed.untagged(HIBIT_SET_VALUE_SIGNED)))
		self.assertNotEqual(hash(unsigned.untagged(HIBIT_UNSET_VALUE)), hash(signed.untagged(HIBIT_UNSET_VALUE)))

		self.assertNotEqual(val_hash, hash(unsigned.untagged(DEFAULT_VALUE - 1)))

		self.assertNotEqual(val_hash, None)

	def test_string_is_supported(self):
		self.assertEqual('0x456789ABCDEF0123', str(BaseValue(WORD_WIDTH, DEFAULT_VALUE)))
		self.assertEqual('0xABCDEF0123456789', str(BaseValue(WORD_WIDTH, HIBIT_SET_VALUE)))
		self.assertEqual('-0x543210FEDCBA9877', str(BaseValue(WORD_WIDTH, HIBIT_SET_VALUE_SIGNED, None, True)))

	def test_string_is_affected_by_width(self):
		self.assertEqual('0x000FEDCBA9876543', str(BaseValue(WORD_WIDTH, 0xFEDCBA9876543)))
		self.assertEqual('0x00000123', str(BaseValue(4, 0x123)))
		self.assertEqual('0x00000000', str(BaseValue(4, 0)))
		self.assertEqual('0x00', str(BaseValue(1, 0)))
