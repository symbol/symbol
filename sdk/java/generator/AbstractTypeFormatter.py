"""Abstract base class shared by every Java-emitting formatter; each returns
:class:`MethodDescriptor` objects that :class:`TypeFormatter` renders into a complete `.java` file."""

from abc import ABC, abstractmethod


class MethodDescriptor:  # pylint: disable=too-many-instance-attributes
	def __init__(self, method_name=None, arguments=None, body='// no body'):
		self.method_name = method_name
		self.arguments = arguments or []
		self.body = body
		self.annotations = []
		self.documentation = []
		self.result = None
		self.visibility = 'public'
		self.is_static = False


class AbstractTypeFormatter(ABC):
	@property
	@abstractmethod
	def typename(self):
		raise NotImplementedError('need to override method')

	@property
	def is_type_abstract(self):
		return False

	def get_base_class(self):
		"""Java ``extends X`` target; return empty string for none."""
		# pylint: disable=no-self-use
		return ''

	def get_implements_clause(self):
		"""Java ``implements X, Y`` clause; return empty string for none."""
		# pylint: disable=no-self-use
		return ''

	def get_permits(self):
		"""Java ``permits X, Y`` list for a sealed hierarchy; a non-empty list flips the class
		declaration to ``public sealed``. The default ``[]`` emits a regular class."""
		# pylint: disable=no-self-use
		return []

	@staticmethod
	def get_class_documentation():
		return None

	@abstractmethod
	def get_ctor_descriptor(self) -> MethodDescriptor:
		pass

	def get_compare_to_descriptor(self) -> MethodDescriptor:  # pylint: disable=no-self-use
		return None

	def get_sort_descriptor(self) -> MethodDescriptor:  # pylint: disable=no-self-use
		return None

	@abstractmethod
	def get_deserialize_descriptor(self) -> MethodDescriptor:
		pass

	@abstractmethod
	def get_serialize_descriptor(self) -> MethodDescriptor:
		pass

	def get_serialize_protected_descriptor(self) -> MethodDescriptor:
		# pylint: disable=no-self-use
		return None

	@abstractmethod
	def get_size_descriptor(self) -> MethodDescriptor:
		pass

	def get_getter_setter_descriptors(self):
		# pylint: disable=no-self-use
		return []

	def get_str_descriptor(self) -> MethodDescriptor:
		# pylint: disable=no-self-use
		return None

	def get_json_descriptor(self) -> MethodDescriptor:
		# pylint: disable=no-self-use
		return None

	def get_fields(self):
		"""Return Java field declarations including modifiers and trailing ``;``."""
		# pylint: disable=no-self-use
		return []

	def get_extra_methods(self):
		"""Return additional fully-rendered method strings (used for static helpers)."""
		# pylint: disable=no-self-use
		return []
