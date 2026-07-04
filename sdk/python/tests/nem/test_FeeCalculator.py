import unittest

from symbolchain.nc import Transaction, TransactionType
from symbolchain.nem.FeeCalculator import calculate_mosaic_rental_fee, calculate_namespace_rental_fee, calculate_transaction_fee
from symbolchain.nem.Network import Network
from symbolchain.nem.TransactionFactory import TransactionFactory


class CalculateMosaicRentalFeeTest(unittest.TestCase):
	def test_calculates_correct_fee(self):
		self.assertEqual(10_000_000, calculate_mosaic_rental_fee())


class CalculateNamespaceRentalFeeTest(unittest.TestCase):
	def test_calculates_correct_root_fee(self):
		self.assertEqual(100_000_000, calculate_namespace_rental_fee(True))

	def test_calculates_correct_child_fee(self):
		self.assertEqual(10_000_000, calculate_namespace_rental_fee(False))


class CalculateTransactionFeeTest(unittest.TestCase):
	@staticmethod
	def _create_transaction_with_type(transaction_type):
		transaction = Transaction()
		transaction.type_ = transaction_type
		return transaction

	def test_calculates_correct_fee_for_multisig_account_modification(self):
		# Act:
		fee = calculate_transaction_fee(self._create_transaction_with_type(TransactionType.MULTISIG_ACCOUNT_MODIFICATION))

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(500_000, fee)

	def test_calculates_correct_fee_for_other_transactions(self):
		# Arrange:
		special_transaction_type_names = ['TRANSFER', 'MULTISIG_ACCOUNT_MODIFICATION']

		def _is_other_transaction_type(property_name):
			return property_name not in special_transaction_type_names and not property_name.startswith('_')

		other_transaction_type_names = list(filter(_is_other_transaction_type, dir(TransactionType)))

		# Sanity:
		self.assertEqual(6, len(other_transaction_type_names))

		# Act:
		for transaction_type_name in other_transaction_type_names:
			fee = calculate_transaction_fee(self._create_transaction_with_type(TransactionType[transaction_type_name]))

			# Assert:
			self.assertIsInstance(fee, int)
			self.assertEqual(150_000, fee)

	@staticmethod
	def _weight_with_fee_unit(amount):
		return amount * 50_000

	@staticmethod
	def _create_mosaic_id(namespace_name, name):
		return {'namespace_id': {'name': namespace_name.encode('utf8')}, 'name': name.encode('utf8')}

	# region transfers - simple

	def _assert_xem_fee(self, amount, message_size, expected_fee):
		# Arrange:
		factory = TransactionFactory(Network.TESTNET)

		descriptor = {
			'type': 'transfer_transaction_v2',
			'amount': amount * 1_000_000
		}
		if message_size:
			descriptor['message'] = {
				'message_type': 1,
				'message': 'a' * message_size
			}

		transaction = factory.create(descriptor)

		# Act:
		fee = calculate_transaction_fee(transaction)

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(expected_fee, fee, f'amount ${amount}, message_size ${message_size}')

	def test_calculates_correct_fee_for_transfers_simple_when_empty(self):
		self._assert_xem_fee(0, 0, self._weight_with_fee_unit(1))

	def test_calculates_correct_fee_for_transfers_simple_near_step_increases(self):
		# Arrange: fee is initially 1 and increased every 10k XEM until is reaches a max fee of 25 XEM
		step = 10_000
		for i in range(26):
			amount = i * step
			fee = max(1, min(25, amount / step))

			# Act + Assert:
			self._assert_xem_fee(amount, 0, self._weight_with_fee_unit(fee))
			self._assert_xem_fee(amount + 1, 0, self._weight_with_fee_unit(fee))
			self._assert_xem_fee(amount + 100, 0, self._weight_with_fee_unit(fee))
			self._assert_xem_fee(amount + step - 1, 0, self._weight_with_fee_unit(fee))

	def test_calculates_correct_fee_for_transfers_simple_caps_fee(self):
		amounts = [250_000, 250_001, 500_000, 1_000_000, 10_000_000, 100_000_000, 1_000_000_000]
		for amount in amounts:
			self._assert_xem_fee(amount, 0, self._weight_with_fee_unit(25))

	def test_calculates_correct_fee_for_transfers_simple_with_message(self):
		self._assert_xem_fee(10_000, 96, self._weight_with_fee_unit(1 + 4))
		self._assert_xem_fee(100_000, 128, self._weight_with_fee_unit(10 + 5))
		self._assert_xem_fee(1_000_000, 96, self._weight_with_fee_unit(25 + 4))
		self._assert_xem_fee(2_000_000, 128, self._weight_with_fee_unit(25 + 5))

	def test_calculates_correct_fee_for_transfers_simple_with_smallest_message(self):
		self._assert_xem_fee(1200, 1, self._weight_with_fee_unit(1 + 1))

	def test_calculates_correct_fee_for_transfers_simple_near_message_step_increases(self):
		self._assert_xem_fee(1200, 31, self._weight_with_fee_unit(1 + 1))
		self._assert_xem_fee(1200, 32, self._weight_with_fee_unit(1 + 2))
		self._assert_xem_fee(1200, 33, self._weight_with_fee_unit(1 + 2))

		self._assert_xem_fee(1200, 63, self._weight_with_fee_unit(1 + 2))
		self._assert_xem_fee(1200, 64, self._weight_with_fee_unit(1 + 3))
		self._assert_xem_fee(1200, 65, self._weight_with_fee_unit(1 + 3))

	def test_calculates_correct_fee_for_transfers_simple_with_large_message(self):
		self._assert_xem_fee(1200, 96, self._weight_with_fee_unit(1 + 4))
		self._assert_xem_fee(1200, 128, self._weight_with_fee_unit(1 + 5))
		self._assert_xem_fee(1200, 256, self._weight_with_fee_unit(1 + 9))
		self._assert_xem_fee(1200, 320, self._weight_with_fee_unit(1 + 11))

	# endregion

	# region transfers - small business mosaics

	# A so-called small business mosaic has divisibility of 0 and a max supply of 10000
	# It is always charged 1 XEM fee no matter how many mosaics are transferred
	# Mosaic 'small business x' has divisibility 0 and supply x * 1000 for x > 0
	# Mosaic 'small business 0' has divisibility 1 and supply 1000 (so it is NOT a small business mosaic)

	def _assert_small_business_mosaic_fee(self, small_business_id, amount, expected_fee):
		def _mosaic_information_lookup(mosaic_id):
			small_business_prefix = 'small business'
			if f'{small_business_prefix} 0' == mosaic_id['name']:
				return {'supply': 1000, 'divisibility': 1}

			if mosaic_id['name'].startswith(small_business_prefix):
				supply = int(mosaic_id['name'][len(small_business_prefix) + 1:]) * 1000
				return {'supply': supply, 'divisibility': 0}

			return None

		# Arrange:
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v2',
			'amount': 1_000_000,
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': self._create_mosaic_id('foo', f'small business {small_business_id}'),
						'amount': amount
					}
				}
			]
		})

		# Act:
		fee = calculate_transaction_fee(transaction, _mosaic_information_lookup)

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(expected_fee, fee, f'small_business_id {small_business_id}')

	def test_calculates_correct_fee_for_transfers_uses_minimum_fee_for_mosaics_with_divisibility_zero_and_low_supply(self):
		for i in range(11):
			self._assert_small_business_mosaic_fee(i, i * 1000, self._weight_with_fee_unit(1))

	def test_calculates_correct_fee_for_transfers_does_not_use_minimum_fee_for_mosaics_with_divisibility_zero_and_supply_above_threshold(self):
		# Assert: supply of 11000 means it is not a small business mosaic
		self._assert_small_business_mosaic_fee(11, 1000, self._weight_with_fee_unit(4))

	def test_calculates_correct_fee_for_transfers_does_not_use_minimum_fee_for_mosaics_with_divisibility_greater_than_zero(self):
		# Arrange: nonzero divisibility means it is not a small business mosaic
		self._assert_small_business_mosaic_fee(0, 1000, self._weight_with_fee_unit(3))

	# endregion

	# region transfers - other mosaic

	# mosaic definition data used for the following tests: supply = 100_000_000, divisibility = 3
	# supply ratio: 8_999_999_999 / 100_000_000 ≈ 90
	# divisibility ratio = 1_000_000 / 1_000 = 1000
	# 1000 / 90 = 11.11..., so transferring a quantity of 12 is roughly like transferring 1 XEM
	# Adjustment for the fee is 9 XEM due to the lower supply and divisibility

	@staticmethod
	def _mosaic_information_lookup_other_mosaic(mosaic_id):
		multiplier = int(mosaic_id['name'])
		divisibility_change = multiplier - 1
		return {'supply': 100_000_000 * multiplier, 'divisibility': 3 + divisibility_change}

	def _assert_single_mosaic_fee(self, amount, message_size, quantity, expected_fee):
		# Arrange:
		factory = TransactionFactory(Network.TESTNET)

		descriptor = {
			'type': 'transfer_transaction_v2',
			'amount': amount * 1_000_000,
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': self._create_mosaic_id('foo', '1'),
						'amount': quantity
					}
				}
			]
		}
		if message_size:
			descriptor['message'] = {
				'message_type': 1,
				'message': 'a' * message_size
			}

		transaction = factory.create(descriptor)

		# Act:
		fee = calculate_transaction_fee(transaction, self._mosaic_information_lookup_other_mosaic)

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(expected_fee, fee, f'amount ${amount}, messageSize ${message_size}, quantity ${quantity}')

	def test_calculates_correct_fee_for_transfers_other_mosaic_near_mosaic_transfer_step_increases(self):
		# Assert: minimum fee for low amounts
		self._assert_single_mosaic_fee(1, 0, 12, self._weight_with_fee_unit(1))  # ~ 1 XEM
		self._assert_single_mosaic_fee(1, 0, 111_000, self._weight_with_fee_unit(1))  # ~9_999 XEM

		# - 1 -> 2 roughly at 1222.222 units
		self._assert_single_mosaic_fee(1, 0, 1_222_000, self._weight_with_fee_unit(1))
		self._assert_single_mosaic_fee(1, 0, 1_223_000, self._weight_with_fee_unit(2))  # ~ 110_000 XEM
		self._assert_single_mosaic_fee(1, 0, 1_224_000, self._weight_with_fee_unit(2))

		# - 2 -> 3 roughly at 1333.333 units
		self._assert_single_mosaic_fee(1, 0, 1_333_000, self._weight_with_fee_unit(2))
		self._assert_single_mosaic_fee(1, 0, 1_334_000, self._weight_with_fee_unit(3))  # ~ 120_000 XEM
		self._assert_single_mosaic_fee(1, 0, 1_335_000, self._weight_with_fee_unit(3))

		# - 3 -> 4 roughly at 1444.444 units
		self._assert_single_mosaic_fee(1, 0, 1_444_000, self._weight_with_fee_unit(3))
		self._assert_single_mosaic_fee(1, 0, 1_445_000, self._weight_with_fee_unit(4))  # ~ 130_000 XEM
		self._assert_single_mosaic_fee(1, 0, 1_446_000, self._weight_with_fee_unit(4))

	def test_calculates_correct_fee_for_transfers_other_mosaic_large_mosaic_transfers(self):
		self._assert_single_mosaic_fee(1, 0, 2_112_000, self._weight_with_fee_unit(10))  # ~ 190_000 XEM
		self._assert_single_mosaic_fee(1, 0, 2_445_000, self._weight_with_fee_unit(13))  # ~ 220_000 XEM
		self._assert_single_mosaic_fee(1, 0, 2_778_000, self._weight_with_fee_unit(16))  # ~ 250_000 XEM
		self._assert_single_mosaic_fee(1, 0, 3_000_000, self._weight_with_fee_unit(16))
		self._assert_single_mosaic_fee(1, 0, 10_000_000, self._weight_with_fee_unit(16))
		self._assert_single_mosaic_fee(1, 0, 100_000_000, self._weight_with_fee_unit(16))

	def test_calculates_correct_fee_for_transfers_other_mosaic_with_amounts_greater_than_one(self):
		# Assert: notice that amount * quantity is constant
		self._assert_single_mosaic_fee(1, 0, 2_112_000, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(2, 0, 1_056_000, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(5, 0, 422_400, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(10, 0, 211_200, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(100, 0, 21_120, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(1_000, 0, 2_112, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(21_120, 0, 100, self._weight_with_fee_unit(10))
		self._assert_single_mosaic_fee(2_112_000, 0, 1, self._weight_with_fee_unit(10))

	def test_calculates_correct_fee_for_transfers_other_mosaic_with_message(self):
		self._assert_single_mosaic_fee(1, 15, 2_112_000, self._weight_with_fee_unit(10 + 1))
		self._assert_single_mosaic_fee(1, 32, 2_112_000, self._weight_with_fee_unit(10 + 2))
		self._assert_single_mosaic_fee(1, 96, 2_112_000, self._weight_with_fee_unit(10 + 4))
		self._assert_single_mosaic_fee(1, 160, 2_112_000, self._weight_with_fee_unit(10 + 6))

	def test_calculates_correct_fee_for_transfers_other_mosaic_sums_fees_when_transferring_several_mosaics_function_based_lookup(self):
		# Arrange: mosaic definitions are (100M, 3), (200M, 4), (300M, 5)
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v2',
			'amount': 1_000_000,
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': self._create_mosaic_id('foo', str(i + 1)),
						'amount': amount
					}
				} for (i, amount) in enumerate([2_000_000, 50_000_000, 800_000_000])
			]
		})

		# Act:
		fee = calculate_transaction_fee(transaction, self._mosaic_information_lookup_other_mosaic)

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(self._weight_with_fee_unit(8 + 16 + 19), fee)

	def test_calculates_correct_fee_for_transfers_other_mosaic_sums_fees_when_transferring_several_mosaics_object_based_lookup(self):
		# Arrange: mosaic definitions are (100M, 3), (200M, 4), (300M, 5)
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v2',
			'amount': 1_000_000,
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': self._create_mosaic_id('foo', str(i + 1)),
						'amount': amount
					}
				} for (i, amount) in enumerate([2_000_000, 50_000_000, 800_000_000])
			]
		})

		# Act:
		fee = calculate_transaction_fee(transaction, {
			'foo:1': {'supply': 100_000_000, 'divisibility': 3},
			'foo:2': {'supply': 200_000_000, 'divisibility': 4},
			'foo:3': {'supply': 300_000_000, 'divisibility': 5}
		})

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(self._weight_with_fee_unit(8 + 16 + 19), fee)

	# endregion

	# region transfers - edge case

	def test_calculates_correct_fee_for_transfers_uses_minimum_fee_when_mosaic_supply_is_zero(self):
		# Arrange:
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v2',
			'amount': 1_000_000,
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': self._create_mosaic_id('foo', 'zero supply'),
						'amount': 5000000
					}
				}
			]
		})

		# Act:
		fee = calculate_transaction_fee(transaction, lambda _: {'supply': 0, 'divisibility': 3})

		# Assert:
		self.assertIsInstance(fee, int)
		self.assertEqual(self._weight_with_fee_unit(1), fee)

	def test_fails_calculating_fee_for_transfers_with_unknown_mosaic(self):
		# Arrange:
		factory = TransactionFactory(Network.TESTNET)
		transaction = factory.create({
			'type': 'transfer_transaction_v2',
			'amount': 1_000_000,
			'mosaics': [
				{
					'mosaic': {
						'mosaic_id': self._create_mosaic_id('foo', 'bar'),
						'amount': 5000000
					}
				}
			]
		})

		# Act + Assert:
		with self.assertRaisesRegex(ValueError, 'unable to find fee information for foo:bar'):
			calculate_transaction_fee(transaction, lambda _: None)

	# endregion
