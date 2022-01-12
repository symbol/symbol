import operator


class ComparisonTestDescriptor:
	def __init__(self, untagged, tagged, fake_value):
		self.untagged = untagged
		self.tagged = tagged
		self.fake_value = fake_value


class EqualityTestDescriptor:
	def __init__(self, untagged, tagged, random, fake_value):
		self.untagged = untagged
		self.tagged = tagged
		self.random = random
		self.fake_value = fake_value


class ComparisonTestUtils:
	# pylint: disable=no-member

	@staticmethod
	def __equality_helper_tag_independent(factory, descriptor, value, checks):
		# Arrange:
		obj = factory(value)
		validate_equal, validate_inequal = checks

		# Act + Assert:
		validate_equal(obj, factory(value))
		validate_inequal(obj, factory(descriptor.random()))
		validate_inequal(obj, descriptor.fake_value(value))
		validate_inequal(obj, None)

	@staticmethod
	def __equality_helper(descriptor, value, checks):
		ComparisonTestUtils.__equality_helper_tag_independent(descriptor.tagged, descriptor, value, checks)
		ComparisonTestUtils.__equality_helper_tag_independent(descriptor.untagged, descriptor, value, checks)

		_, validate_inequal = checks
		validate_inequal(descriptor.untagged(value), descriptor.tagged(value))

	def equality_is_supported(self, descriptor, value):
		self.__equality_helper(descriptor, value, [
			lambda a, b: self.assertTrue(operator.eq(a, b)),
			lambda a, b: self.assertFalse(operator.eq(a, b)),
		])

	def inequality_is_supported(self, descriptor, value):
		self.__equality_helper(descriptor, value, [
			lambda a, b: self.assertFalse(operator.ne(a, b)),
			lambda a, b: self.assertTrue(operator.ne(a, b)),
		])

	def __order_helper(self, descriptor, values, operation, assert_equality=False):
		value = descriptor.untagged(values[1])

		# Act + Assert:
		self.assertTrue(operation(value, descriptor.untagged(values[2])))
		self.assertTrue(operation(descriptor.untagged(values[0]), value))

		assertion = self.assertTrue if assert_equality else self.assertFalse
		assertion(operation(value, value))
		assertion(operation(value, descriptor.untagged(values[1])))

		self.assertFalse(operation(value, descriptor.untagged(values[0])))
		self.assertFalse(operation(descriptor.untagged(values[2]), value))
		self.assertFalse(operation(value, descriptor.tagged(values[2])))
		with self.assertRaises(TypeError):
			_ = operation(value, descriptor.fake_value(values[1]))

		with self.assertRaises(TypeError):
			_ = operation(value < None)

	def less_than_is_supported(self, descriptor, reference_value):
		ordered_values = [reference_value - 1, reference_value, reference_value + 1]
		self.__order_helper(descriptor, ordered_values, operator.lt)

	def less_than_equal_is_supported(self, descriptor, reference_value):
		ordered_values = [reference_value - 1, reference_value, reference_value + 1]
		self.__order_helper(descriptor, ordered_values, operator.le, True)

	def greater_than_is_supported(self, descriptor, reference_value):
		ordered_values = [reference_value + 1, reference_value, reference_value - 1]
		self.__order_helper(descriptor, ordered_values, operator.gt)

	def greater_than_equal_is_supported(self, descriptor, reference_value):
		ordered_values = [reference_value + 1, reference_value, reference_value - 1]
		self.__order_helper(descriptor, ordered_values, operator.ge, True)
