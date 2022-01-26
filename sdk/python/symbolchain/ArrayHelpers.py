from __future__ import annotations

from typing import Callable, List, Optional, Sequence

from typing_extensions import Protocol


class Serializable(Protocol):
	def serialize(self) -> bytes:
		return bytes()

	def deserialize(self, binary: memoryview) -> Serializable:
		raise RuntimeError('protocol class')

	def size(self) -> int:
		return 0


class ArrayHelpers:
	@staticmethod
	def get_bytes(binary: memoryview, size: int) -> bytes:
		if size > len(binary):
			raise Exception(f'size should not exceed {len(binary)}. The value of size was: {size}')
		return binary[0:size].tobytes()

	@staticmethod
	def read_array_count(
		binary: memoryview,
		element_type: Serializable,
		count: int,
		accessor: Optional[Callable[[Serializable], int]] = None,
	) -> List[Serializable]:
		elements = []
		prev_element = None
		for _ in range(count):
			element = element_type.deserialize(binary)
			if accessor and prev_element:
				assert accessor(prev_element) < accessor(element), 'array is not sorted'

			elements.append(element)
			binary = binary[element.size:]

			prev_element = element
		return elements

	@staticmethod
	def write_array_count(
		elements: Sequence[Serializable],
		count: int,
		accessor: Optional[Callable[[Serializable], int]] = None,
	) -> bytes:
		binary = bytes()
		for i in range(count):
			if accessor and i > 0:
				assert accessor(elements[i - 1]) < accessor(elements[i]), 'array is not sorted'

			binary += elements[i].serialize()
		return binary

	@staticmethod
	def read_array(
		binary: memoryview,
		element_type: Serializable,
		accessor: Optional[Callable[[Serializable], int]] = None,
	) -> List[Serializable]:
		elements = []
		prev_element = None
		# note: this method is used only for '__FILL__' type arrays,
		# this loop assumes, proper binary buffer slice is passed and that there's
		# no additional data
		# In generated code this is done by limiting buffer, when 'size' field is read.
		while len(binary) > 0:
			element = element_type.deserialize(binary)
			if accessor and prev_element:
				assert accessor(prev_element) < accessor(element), 'array is not sorted'

			elements.append(element)
			binary = binary[element.size:]

			prev_element = element
		return elements

	@staticmethod
	def write_array(
		elements: Sequence[Serializable],
		accessor: Optional[Callable[[Serializable], int]] = None,
	) -> bytes:
		binary = bytes()
		i = 0
		for element in elements:
			if accessor and i > 0:
				assert accessor(elements[i - 1]) < accessor(elements[i]), 'array is not sorted'

			binary += element.serialize()
			i += 1

		return binary

	@staticmethod
	def align_up(size: int, alignment: int) -> int:
		return (size + alignment - 1) // alignment * alignment

	@staticmethod
	def read_variable_size_elements(binary: memoryview, factory_type: Serializable, alignment: int) -> List[Serializable]:
		elements = []
		while len(binary) > 0:
			element = factory_type.deserialize(binary)
			elements.append(element)
			embedded_size = element.size
			assert embedded_size > 0

			aligned_size = ArrayHelpers.align_up(embedded_size, alignment)
			binary = binary[aligned_size:]

		return elements

	@staticmethod
	def write_variable_size_elements(elements: Sequence[Serializable], alignment: int) -> bytes:
		binary = bytes()
		for element in elements:
			binary += element.serialize()

			embedded_size = element.size
			aligned_size = ArrayHelpers.align_up(embedded_size, alignment)
			binary += bytes(aligned_size - embedded_size)
		return binary
