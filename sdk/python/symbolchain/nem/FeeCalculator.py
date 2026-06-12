import math

from symbolchain.nc import TransactionType


def calculate_mosaic_rental_fee():
	"""Calculates the minimum required mosaic rental fee."""

	return 10 * 1_000_000


def calculate_namespace_rental_fee(is_root):
	"""
	Calculates the minimum required namespace rental fee.
	When is_root is True, calculates the root namespace fee.
	Otherwise, calculates the child namespace fee.
	"""

	return (100 if is_root else 10) * 1_000_000


def _decode_mosaic_id(mosaic_id):
	return {'namespace_id': {'name': mosaic_id.namespace_id.name.decode('utf8')}, 'name': mosaic_id.name.decode('utf8')}


def _calculate_unweighted_transfer_fee(transaction, mosaic_information_lookup):
	xem_supply = 8_999_999_999
	max_mosaic_units = 9_000_000_000_000_000

	def _calculate_xem_transfer_fee(amount):
		return int(min(25, max(1, amount // 10000)))

	def _calculate_mosaic_total_quantity(mosaic_information):
		return mosaic_information['supply'] * (10 ** mosaic_information['divisibility'])

	def _calculate_xem_equivalent(amount, mosaic_amount, mosaic_information):
		if 0 == mosaic_information['supply']:
			return 0

		# amount          XEM whole units
		# mosaic_amount   mosaic atomic units
		# xem_supply / _calculate_mosaic_total_quantity(mosaic_information)  convert mosaic_amount from mosaic units to XEM equivalent units
		return amount * mosaic_amount * xem_supply / _calculate_mosaic_total_quantity(mosaic_information)

	def _calculate_mosaic_transfer_fee(amount, mosaic, mosaic_information):
		if 0 == mosaic_information['divisibility'] and 10_000 >= mosaic_information['supply']:
			return 1

		xem_equivalent = _calculate_xem_equivalent(amount, mosaic.mosaic.amount.value, mosaic_information)
		xem_fee = _calculate_xem_transfer_fee(xem_equivalent)
		mosaic_total_quantity = _calculate_mosaic_total_quantity(mosaic_information)
		supply_related_adjustment = 0 if 0 == mosaic_total_quantity else int(0.8 * math.log(max_mosaic_units / mosaic_total_quantity))
		return max(1, xem_fee - supply_related_adjustment)

	message_fee = 0 if not transaction.message else len(transaction.message.message) // 32 + 1
	amount = transaction.amount.value // 1_000_000  # convert to XEM whole units
	if not transaction.mosaics:
		transfer_fee = _calculate_xem_transfer_fee(amount)
		return message_fee + transfer_fee

	def _lookup_and_calculate_mosaic_transfer_fee(mosaic):
		mosaic_id = _decode_mosaic_id(mosaic.mosaic.mosaic_id)
		mosaic_information = mosaic_information_lookup(mosaic_id)
		if not mosaic_information:
			raise ValueError(f'unable to find fee information for {mosaic_id["namespace_id"]["name"]}:{mosaic_id["name"]}')

		return _calculate_mosaic_transfer_fee(amount, mosaic, mosaic_information)

	transfer_fee = sum(_lookup_and_calculate_mosaic_transfer_fee(mosaic) for mosaic in transaction.mosaics)
	return message_fee + transfer_fee


def calculate_transaction_fee(transaction, mosaic_information_lookup=None):
	"""
	Calculates the minimum required transaction fee for a transaction.
	mosaic_information_lookup looks up mosaic information ({supply, divisibility}) given mosaic identifier.
	When a function, mosaic identifier will be passed as parameter.
	When an object map, fully qualified mosaic identifier will be used as an index.
	When None, this function will be unable to calculate fees for custom mosaic transfers.
	"""

	def _weight_with_fee_unit(amount):
		return amount * 50_000

	if TransactionType.TRANSFER != transaction.type_:
		return _weight_with_fee_unit(10 if TransactionType.MULTISIG_ACCOUNT_MODIFICATION == transaction.type_ else 3)

	def _make_lookup_function(lookup):
		if callable(lookup):
			return lookup

		return lambda mosaic_id: lookup[f'{mosaic_id["namespace_id"]["name"]}:{mosaic_id["name"]}']

	return _weight_with_fee_unit(_calculate_unweighted_transfer_fee(transaction, _make_lookup_function(mosaic_information_lookup)))
