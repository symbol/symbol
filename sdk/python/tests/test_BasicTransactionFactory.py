import unittest

from symbolchain.BasicTransactionFactory import BasicTransactionFactory
from symbolchain.CryptoTypes import PublicKey


class FakeNetwork:
	pass


class MockTransaction:
	TYPE_HINTS = {
		'type': 'pod:general_parser',
		'signer_public_key': 'pod:general_parser',
		'breadcrumb_1': 'pod:general_parser'
		# note: deliberately no hint for breadcrumb_2
	}

	def __init__(self):
		self.type = None
		self.network = None
		self.signer_public_key = None
		self.breadcrumb_1 = None
		self.breadcrumb_2 = None


class MockTransactionFactory(BasicTransactionFactory):
	NETWORK = FakeNetwork()

	def __init__(self):
		super().__init__(
			self.NETWORK,
			lambda value: f'converted_{value}',
			{
				'general_parser': lambda value: self.general_parser_breadcrumbs.append(f'general_{value}') or value,
				PublicKey: lambda value: self.public_key_parser_breadcrumbs.append(f'public_{value}') or value,
			})

		self.public_key_parser_breadcrumbs = []
		self.general_parser_breadcrumbs = []
		self.typenames_breadcrumbs = []

	def create_by_name(self, typename):
		self.typenames_breadcrumbs.append(typename)
		return MockTransaction()

	def create(self, descriptor):
		# pass self as transaction factory
		return self._create(descriptor, self)


class BasicTransactionFactoryTest(unittest.TestCase):
	def test_factory_requires_type(self):
		# Arrange:
		factory = MockTransactionFactory()

		# Act + Assert:
		with self.assertRaises(ValueError) as context:
			factory.create({})

		self.assertTrue('transaction descriptor does not have attribute type' in str(context.exception))

	def test_factory_requires_type_and_signer_public_key(self):
		# Arrange:
		factory = MockTransactionFactory()

		# Act + Assert:
		with self.assertRaises(ValueError) as context:
			factory.create({'type': 'one'})

		self.assertTrue('transaction descriptor does not have attribute signer_public_key' in str(context.exception))

	def test_factory_does_not_copy_type(self):
		# Arrange:
		factory = MockTransactionFactory()

		# Act:
		transaction = factory.create({'type': 'one', 'signer_public_key': 'two'})

		# Assert:
		self.assertEqual(['converted_one'], factory.typenames_breadcrumbs)

		self.assertIsNone(transaction.type)
		self.assertEqual(MockTransactionFactory.NETWORK, transaction.network)
		self.assertEqual('converted_two', transaction.signer_public_key)

	def test_factory_ignores_transaction_hints_for_both_type_and_signer_public_key(self):
		# Arrange:
		factory = MockTransactionFactory()

		# Act:
		transaction = factory.create({'type': 'one', 'signer_public_key': 'two'})

		# Assert:
		self.assertEqual(['converted_one'], factory.typenames_breadcrumbs)
		self.assertEqual(['public_two'], factory.public_key_parser_breadcrumbs)
		self.assertEqual([], factory.general_parser_breadcrumbs)

		self.assertEqual('converted_two', transaction.signer_public_key)
		self.assertEqual(None, transaction.breadcrumb_1)
		self.assertEqual(None, transaction.breadcrumb_2)

	def test_factory_applies_transaction_hints_for_other_fields(self):
		# Arrange:
		factory = MockTransactionFactory()
		transaction = factory.create({'type': 'one', 'signer_public_key': 'two', 'breadcrumb_1': 'three', 'breadcrumb_2': 'four'})

		# Assert:
		self.assertEqual(['converted_one'], factory.typenames_breadcrumbs)
		self.assertEqual(['public_two'], factory.public_key_parser_breadcrumbs)
		# - parser has only been called for breadcrumb_1
		self.assertEqual(['general_three'], factory.general_parser_breadcrumbs)

		self.assertEqual('converted_two', transaction.signer_public_key)
		self.assertEqual('converted_three', transaction.breadcrumb_1)
		self.assertEqual('converted_four', transaction.breadcrumb_2)
