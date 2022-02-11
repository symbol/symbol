def read_array_impl(view, factory_class, accessor, should_continue):
	elements = []
	previous_element = None

	i = 0
	while should_continue(i, view):
		element = factory_class.deserialize(view)

		if element.size <= 0:
			raise ValueError('element size has invalid size')

		if accessor and previous_element and accessor(previous_element) >= accessor(element):
			raise ValueError('elements in array are not sorted')

		elements.append(element)
		view = view[element.size:]

		previous_element = element
		i += 1

	return elements


def write_array_impl(elements, count, accessor):
	output_buffer = bytes()
	for i in range(0, count):
		element = elements[i]
		if accessor and i > 0 and accessor(elements[i - 1]) >= accessor(element):
			raise ValueError('array passed to write array is not sorted')

		output_buffer += element.serialize()

	return output_buffer


class ArrayHelpers:
	@staticmethod
	def get_bytes(view, size):
		"""Returns first size bytes of view."""
		if size > len(view):
			raise ValueError(f'size should not exceed {len(view)}. The value of size was: {size}.')

		return view[:size].tobytes()

	@staticmethod
	def align_up(size, alignment):
		"""Calculates aligned size."""
		return (size + alignment - 1) // alignment * alignment

	@staticmethod
	def read_array(view, factory_class, accessor=None):
		"""Reads array of objects."""
		return read_array_impl(view, factory_class, accessor, lambda _, view: len(view) > 0)

	@staticmethod
	def read_array_count(view, factory_class, count, accessor=None):
		"""Reads array of deterministic number of objects."""
		return read_array_impl(view, factory_class, accessor, lambda index, _: count > index)

	@staticmethod
	def read_variable_size_elements(view, factory_class, alignment):
		"""Reads array of variable size objects."""
		elements = []
		while len(view) > 0:
			element = factory_class.deserialize(view)

			if element.size <= 0:
				raise ValueError('element size has invalid size')

			elements.append(element)

			aligned_size = ArrayHelpers.align_up(element.size, alignment)
			if aligned_size > len(view):
				raise ValueError('unexpected buffer length')

			view = view[aligned_size:]

		return elements

	@staticmethod
	def write_array(elements, accessor=None):
		"""Writes array of objects."""
		return write_array_impl(elements, len(elements), accessor)

	@staticmethod
	def write_array_count(elements, count, accessor=None):
		"""Writes array of deterministic number of objects."""
		return write_array_impl(elements, count, accessor)

	@staticmethod
	def write_variable_size_elements(elements, alignment):
		"""Writes array of variable size objects."""
		output_buffer = bytes()
		for element in elements:
			output_buffer += element.serialize()

			aligned_size = ArrayHelpers.align_up(element.size, alignment)
			if aligned_size != element.size:
				output_buffer += bytes(aligned_size - element.size)

		return output_buffer
