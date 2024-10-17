from symbolchain import sc
from symbolchain.RuleBasedTransactionFactory import RuleBasedTransactionFactory
from symbolchain.symbol.Network import Address


class ReceiptFactory:
	"""Factory for creating Symbol receipts."""

	def __init__(self, type_rule_overrides=None):
		"""Creates a factory."""
		self.factory = self._build_rules(type_rule_overrides)

	def create(self, receipt_descriptor):
		"""Creates a receipt from a receipt_descriptor."""
		return self.factory.create_from_factory(sc.ReceiptFactory.create_by_name, receipt_descriptor), receipt_descriptor

	@staticmethod
	def _build_rules(type_rule_overrides):
		factory = RuleBasedTransactionFactory(sc, None, type_rule_overrides)
		factory.autodetect()

		factory.add_struct_parser('Mosaic')

		sdk_type_mapping = {
			'Address': Address,
		}
		for name, typename in sdk_type_mapping.items():
			factory.add_pod_parser(name, typename)

		return factory
