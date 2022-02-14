import unittest
from collections import namedtuple

from symbolchain.ArrayHelpers import ArrayHelpers

from .test.TestUtils import TestUtils

DeserializedTuple = namedtuple('DeserializedTuple', ['size', 'tag'])


class ArrayHelpersTest(unittest.TestCase):
	# pylint: disable=too-many-public-methods

	# region get_bytes

	def _assert_get_bytes_can_return_subview(self, size, expected):
		# Arrange:
		backing = b'this is a test'
		view = memoryview(backing)

		# Act:
		subview = ArrayHelpers.get_bytes(view, size)

		# Assert:
		self.assertEqual(expected, subview)

	def test_get_bytes_can_return_partial_subview(self):
		self._assert_get_bytes_can_return_subview(4, b'this')

	def test_get_bytes_can_return_full_subview(self):
		self._assert_get_bytes_can_return_subview(14, b'this is a test')

	def test_get_bytes_cannot_return_more_bytes_than_in_view(self):
		# Arrange:
		backing = b'this is a test'
		view = memoryview(backing)

		# Act + Assert:
		for size in [15, 100]:
			with self.assertRaises(ValueError):
				ArrayHelpers.get_bytes(view, size)

	# endregion

	# region align_up

	def _assert_align_up(self, range_tuple, alignment, expected_value):
		for i in range(range_tuple[0], range_tuple[1] + 1):
			# Act:
			value = ArrayHelpers.align_up(i, alignment)

			# Assert:
			self.assertEqual(expected_value, value)

	def test_align_up_always_aligns_up(self):
		self._assert_align_up((0, 0), 8, 0)
		self._assert_align_up((1, 8), 8, 8)
		self._assert_align_up((9, 16), 8, 16)
		self._assert_align_up((257, 264), 8, 264)

	def test_align_up_can_align_using_custom_alignment(self):
		self._assert_align_up((0, 0), 11, 0)
		self._assert_align_up((1, 11), 11, 11)
		self._assert_align_up((12, 22), 11, 22)
		self._assert_align_up((353, 363), 11, 363)

	# endregion

	# region readers - test utils

	class ReadTestContext:
		def __init__(self, sizes, view_size=52):
			self.sizes = sizes
			self.buffer = TestUtils.randbytes(100)
			self.sub_view = memoryview(self.buffer)[15:15 + view_size]
			self.index = 0

		def deserialize(self, buffer):
			index = self.buffer.index(buffer)
			result = DeserializedTuple(self.sizes[self.index], index)
			self.index += 1
			return result

	def _assert_reader_throws_when_any_element_has_zero_size(self, reader):
		# Arrange:
		context = self.ReadTestContext([10, 11, 0, 1, 1])

		# Act + Assert:
		with self.assertRaises(ValueError):
			reader(context.sub_view, context)

	def _assert_reader_reads_all_available_elements(self, reader, sizes, expected_elements):
		# Arrange:
		context = self.ReadTestContext(sizes)

		# Act:
		elements = reader(context.sub_view, context)

		# Assert:
		self.assertEqual(expected_elements, elements)

	def _assert_reader_can_read_when_using_accessor_and_elements_are_ordered(self, reader, sizes, expected_elements):
		# Arrange:
		context = self.ReadTestContext(sizes)

		# Act:
		elements = reader(context.sub_view, context, lambda element: element.tag)

		# Assert:
		self.assertEqual(expected_elements, elements)

	def _assert_reader_cannot_read_when_using_accessor_and_elements_are_not_ordered(self, reader, sizes):
		# Arrange:
		context = self.ReadTestContext(sizes)

		# Act + Assert:
		with self.assertRaises(ValueError):
			reader(context.sub_view, context, lambda element: -element.tag)

	# endregion

	# region readers - read_array

	class ReadArrayTraits:
		def __init__(self):
			self.sizes = [10, 11, 12, 13, 6]
			self.expected_elements = [
				DeserializedTuple(10, 15),
				DeserializedTuple(11, 25),
				DeserializedTuple(12, 36),
				DeserializedTuple(13, 48),
				DeserializedTuple(6, 61)
			]
			self.reader = ArrayHelpers.read_array

	def test_read_array_throws_when_any_element_has_zero_size(self):
		traits = self.ReadArrayTraits()
		self._assert_reader_throws_when_any_element_has_zero_size(traits.reader)

	def test_read_array_reads_all_available_elements(self):
		traits = self.ReadArrayTraits()
		self._assert_reader_reads_all_available_elements(traits.reader, traits.sizes, traits.expected_elements)

	def test_read_array_can_read_when_using_accessor_and_elements_are_ordered(self):
		traits = self.ReadArrayTraits()
		self._assert_reader_can_read_when_using_accessor_and_elements_are_ordered(traits.reader, traits.sizes, traits.expected_elements)

	def test_read_array_cannot_read_when_using_accessor_and_elements_are_not_ordered(self):
		traits = self.ReadArrayTraits()
		self._assert_reader_cannot_read_when_using_accessor_and_elements_are_not_ordered(traits.reader, traits.sizes)

	# endregion

	# region readers - read_array_count

	class ReadArrayCountTraits:
		def __init__(self):
			self.sizes = [10, 11, 12, 43, 79]
			self.expected_elements = [
				DeserializedTuple(10, 15),
				DeserializedTuple(11, 25),
				DeserializedTuple(12, 36)
			]

		@staticmethod
		def reader(view, factory_class, accessor=None):
			return ArrayHelpers.read_array_count(view, factory_class, 3, accessor)

	def test_read_array_count_throws_when_any_element_has_zero_size(self):
		traits = self.ReadArrayCountTraits()
		self._assert_reader_throws_when_any_element_has_zero_size(traits.reader)

	def test_read_array_count_reads_all_available_elements(self):
		traits = self.ReadArrayCountTraits()
		self._assert_reader_reads_all_available_elements(traits.reader, traits.sizes, traits.expected_elements)

	def test_read_array_count_can_read_when_using_accessor_and_elements_are_ordered(self):
		traits = self.ReadArrayCountTraits()
		self._assert_reader_can_read_when_using_accessor_and_elements_are_ordered(traits.reader, traits.sizes, traits.expected_elements)

	def test_read_array_count_cannot_read_when_using_accessor_and_elements_are_not_ordered(self):
		traits = self.ReadArrayCountTraits()
		self._assert_reader_cannot_read_when_using_accessor_and_elements_are_not_ordered(traits.reader, traits.sizes)

	# endregion

	# region readers - read_variable_size_elements

	def test_read_variable_size_elements_throws_when_any_element_has_zero_size(self):
		def reader(view, factory_class):
			ArrayHelpers.read_variable_size_elements(view, factory_class, 4)

		self._assert_reader_throws_when_any_element_has_zero_size(reader)

	def test_read_variable_size_elements_reads_all_available_elements(self):
		# Arrange: aligned sizes 8, 12, 12, 16, 4
		context = self.ReadTestContext([7, 11, 12, 13, 3])
		expected_elements = [
			DeserializedTuple(7, 15),
			DeserializedTuple(11, 15 + 8),
			DeserializedTuple(12, 15 + 8 + 12),
			DeserializedTuple(13, 15 + 8 + 12 + 12),
			DeserializedTuple(3, 15 + 8 + 12 + 12 + 16)
		]

		# Act:
		elements = ArrayHelpers.read_variable_size_elements(context.sub_view, context, 4)

		# Assert:
		self.assertEqual(expected_elements, elements)

	def test_read_variable_size_elements_throws_when_reading_would_result_in_oob_read(self):
		# Arrange:
		context = self.ReadTestContext([24, 25], 49)

		# Act + Assert:
		with self.assertRaises(ValueError):
			ArrayHelpers.read_variable_size_elements(context.sub_view, context, 4)

	# endregion

	# region writers - test utils

	class WriteTestContext:
		class MockElement:
			def __init__(self, size):
				self.size = size

			def serialize(self):
				return bytes([100 + self.size])

		def __init__(self):
			self.elements = [self.MockElement(i * 3 + 1) for i in range(0, 5)]

	def _assert_writer_writes_all_elements(self, writer, expected_output):
		# Arrange:
		context = self.WriteTestContext()

		# Act:
		output = writer(context.elements)

		# Assert:
		self.assertEqual(expected_output, output)

	def _assert_writer_can_write_when_using_accessor_and_elements_are_ordered(self, writer, expected_output):
		# Arrange:
		context = self.WriteTestContext()

		# Act:
		output = writer(context.elements, lambda element: element.size)

		# Assert:
		self.assertEqual(expected_output, output)

	def _assert_writer_cannot_write_when_using_accessor_and_elements_are_not_ordered(self, writer):
		# Arrange:
		context = self.WriteTestContext()

		# Act + Assert:
		with self.assertRaises(ValueError):
			writer(context.elements, lambda element: -element.size)

	# endregion

	# region writers - write_array

	class WriteArrayTraits:
		def __init__(self):
			self.expected_output = bytes([101, 104, 107, 110, 113])
			self.writer = ArrayHelpers.write_array

	def test_write_array_writes_all_elements(self):
		traits = self.WriteArrayTraits()
		self._assert_writer_writes_all_elements(traits.writer, traits.expected_output)

	def test_write_array_can_write_when_using_accessor_and_elements_are_ordered(self):
		traits = self.WriteArrayTraits()
		self._assert_writer_can_write_when_using_accessor_and_elements_are_ordered(traits.writer, traits.expected_output)

	def test_write_array_cannot_write_when_using_accessor_and_elements_are_not_ordered(self):
		traits = self.WriteArrayTraits()
		self._assert_writer_cannot_write_when_using_accessor_and_elements_are_not_ordered(traits.writer)

	# endregion

	# region writers - write_array_count

	class WriteArrayCountTraits:
		def __init__(self):
			self.expected_output = bytes([101, 104, 107])

		@staticmethod
		def writer(elements, accessor=None):
			return ArrayHelpers.write_array_count(elements, 3, accessor)

	def test_write_array_count_writes_all_elements(self):
		traits = self.WriteArrayCountTraits()
		self._assert_writer_writes_all_elements(traits.writer, traits.expected_output)

	def test_write_array_count_can_write_when_using_accessor_and_elements_are_ordered(self):
		traits = self.WriteArrayCountTraits()
		self._assert_writer_can_write_when_using_accessor_and_elements_are_ordered(traits.writer, traits.expected_output)

	def test_write_array_count_cannot_write_when_using_accessor_and_elements_are_not_ordered(self):
		traits = self.WriteArrayCountTraits()
		self._assert_writer_cannot_write_when_using_accessor_and_elements_are_not_ordered(traits.writer)

	# endregion

	# region writers - write_variable_size_elements

	def test_write_variable_size_elements_writes_all_elements_and_aligns(self):
		# Arrange:
		context = self.WriteTestContext()

		# Act:
		output = ArrayHelpers.write_variable_size_elements(context.elements, 4)

		# Assert: notice that alignment is calculated from reported size, not serialized size
		# * 101 - aligned up to 104: [101, 0, 0, 0]
		# * 104 - aligned up to 104: [104]
		# * 107 - aligned up to 108: [107, 0]
		# * 110 - aligned up to 102: [110, 0, 0]
		# * 113 - aligned up to 106: [113, 0, 0, 0]
		self.assertEqual(bytes([101, 0, 0, 0, 104, 107, 0, 110, 0, 0, 113, 0, 0, 0]), output)

	# endregion
