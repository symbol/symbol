from symbolchain.sc import (
	Address,
	AddressResolutionEntry,
	AddressResolutionStatement,
	Amount,
	BlockStatement,
	MosaicExpiredReceipt,
	MosaicId,
	MosaicResolutionEntry,
	MosaicResolutionStatement,
	NamespaceRentalFeeReceipt,
	TransactionStatement,
	UnresolvedAddress,
	UnresolvedMosaicId
)
from symbolchain.symbol.Network import Address as SymbolAddress

SAMPLE_ADDRESS = SymbolAddress('TD2ASJ2LKL5LX66PPZ67PYQN4HIMH5SX7OCZLQI')
SAMPLE_ADDRESS_2 = SymbolAddress('TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ')

SAMPLE_UNRESOLVED_ADDRESS = SymbolAddress('TFJPVO73Z57H35YAAAAAAAAAAAAAAAAAAAAAAAA')


def create_rental_fee_receipt():
	receipt = NamespaceRentalFeeReceipt()
	receipt.mosaic.mosaic_id = MosaicId(0x1234567890ABCDEF)
	receipt.mosaic.amount = Amount(42)
	receipt.sender_address = Address(SAMPLE_ADDRESS.bytes)
	receipt.recipient_address = Address(SAMPLE_ADDRESS_2.bytes)
	return receipt


def create_mosaic_expired_receipt():
	receipt = MosaicExpiredReceipt()
	receipt.artifact_id = MosaicId(0x8877665544332211)
	return receipt


def create_transaction_statement(statement_id=0):
	statement = TransactionStatement()
	statement.primary_id = 0x12345678 + statement_id
	statement.secondary_id = 0x88776655
	statement.receipts.append(create_rental_fee_receipt())
	statement.receipts.append(create_mosaic_expired_receipt())
	return statement


def create_mosaic_resolution_statement(statement_id=0):
	statement = MosaicResolutionStatement()
	statement.unresolved = UnresolvedMosaicId(0x8877665544332211)

	for index in range(3):
		entry = MosaicResolutionEntry()
		entry.source.primary_id = 13 + statement_id
		entry.source.secondary_id = 41 + index * 2
		entry.resolved_value = MosaicId(0x1234567890ABCDEF + index * 3)
		statement.resolution_entries.append(entry)

	return statement


def create_address_resolution_statement(statement_id=0):
	statement = AddressResolutionStatement()
	statement.unresolved = UnresolvedAddress(SAMPLE_UNRESOLVED_ADDRESS.bytes)

	for index, address in enumerate([SAMPLE_ADDRESS, SAMPLE_ADDRESS_2]):
		entry = AddressResolutionEntry()
		entry.source.primary_id = 42 + statement_id
		entry.source.secondary_id = 13 + index * 2
		entry.resolved_value = Address(address.bytes)

	return statement


def create_block_statement():
	statement = BlockStatement()

	for i in range(3):
		statement.transaction_statements.append(create_transaction_statement(i))

	for i in range(4):
		statement.address_resolution_statements.append(create_address_resolution_statement(i))

	for i in range(5):
		statement.mosaic_resolution_statements.append(create_mosaic_resolution_statement(i))

	return statement


other = [
	{
		'schema_name': 'TransactionStatement',
		'descriptor': {
			'type': 'transaction_statement',
			'object': create_transaction_statement()
		}
	},
	{
		'schema_name': 'MosaicResolutionStatement',
		'descriptor': {
			'type': 'mosaic_resolution_statement',
			'object': create_mosaic_resolution_statement()
		}
	},
	{
		'schema_name': 'AddressResolutionStatement',
		'descriptor': {
			'type': 'address_resolution_statement',
			'object': create_address_resolution_statement()
		}
	},
	{
		'schema_name': 'BlockStatement',
		'descriptor': {
			'type': 'block_statement',
			'object': create_block_statement()
		}
	}
]
