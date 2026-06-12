import unittest

from symbolchain.symbol.FeeCalculator import calculate_transaction_fee
from symbolchain.symbol.Network import Network
from symbolchain.symbol.TransactionFactory import TransactionFactory


class CalculateTransactionFeeTest(unittest.TestCase):
	def test_can_calculate_fee_for_transaction_without_cosignatures(self):
		# Arrange:
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v1'
		})

		# Act + Assert: transfer size is 160
		self.assertEqual(16000, calculate_transaction_fee(transaction, 100))
		self.assertEqual(24000, calculate_transaction_fee(transaction, 150))
		self.assertEqual(32000, calculate_transaction_fee(transaction, 200))

	def test_can_calculate_fee_for_transaction_with_cosignatures(self):
		# Arrange:
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v1'
		})

		# Act + Assert: transfer size is 160, cosignature size is 104
		self.assertEqual(16000 + 312, calculate_transaction_fee(transaction, 100, 3))
		self.assertEqual(24000 + 416, calculate_transaction_fee(transaction, 150, 4))
		self.assertEqual(32000 + 520, calculate_transaction_fee(transaction, 200, 5))
