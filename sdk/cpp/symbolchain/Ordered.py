import operator
from abc import ABC, abstractmethod


class Ordered(ABC):
	@abstractmethod
	def _cmp(self, other, operation):
		pass

	def __lt__(self, other):
		return self._cmp(other, operator.lt)

	def __le__(self, other):
		return self._cmp(other, operator.le)

	def __gt__(self, other):
		return self._cmp(other, operator.gt)

	def __ge__(self, other):
		return self._cmp(other, operator.ge)
