from abc import ABC, abstractmethod

from generator.AbstractTypeFormatter import MethodDescriptor


class AbstractTypeFormatter(ABC):
	@property
	@abstractmethod
	def typename(self):
		raise NotImplementedError('need to override method')

	def get_base_class(self):
		# pylint: disable=no-self-use
		return ''

	@abstractmethod
	def get_ctor_descriptor(self) -> MethodDescriptor:
		pass

	@abstractmethod
	def get_to_map_descriptor(self) -> MethodDescriptor:
		pass
