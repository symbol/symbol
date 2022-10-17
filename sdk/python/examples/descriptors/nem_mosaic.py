from symbolchain.facade.NemFacade import Address
from symbolchain.nc import Address as sc_Address
from symbolchain.nc import Amount, MosaicId, MosaicLevy, MosaicTransferFeeType, NamespaceId


def descriptor_factory():
	sample_address = 'TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C'

	# HACK: until fixed
	levy = MosaicLevy()
	levy.transfer_fee_type = MosaicTransferFeeType.ABSOLUTE
	levy.recipient_address = sc_Address(str(Address(sample_address)).encode('utf8'))
	levy.mosaic_id = MosaicId()
	levy.mosaic_id.namespace_id = NamespaceId()
	levy.mosaic_id.namespace_id.name = 'lieutenant'.encode('utf8')
	levy.mosaic_id.name = 'colonel'.encode('utf8')
	levy.fee = Amount(632_0000)

	return [
		# without properties
		{
			'type': 'mosaic_definition_transaction_v1',
			'rental_fee_sink': 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
			'rental_fee': 50000 * 1000000,

			'mosaic_definition': {
				'owner_public_key': '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				'id': {'namespace_id': {'name': 'genes'.encode('utf8')}, 'name': 'memes'.encode('utf8')},
				'description': 'Not really valuable mosaic'.encode('utf8'),
				'properties': [
				],
				'levy': {}
			}
		},

		# with properties
		{
			'type': 'mosaic_definition_transaction_v1',
			'rental_fee_sink': 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
			'rental_fee': 50000 * 1000000,

			'mosaic_definition': {
				'owner_public_key': '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				'id': {'namespace_id': {'name': 'genes'.encode('utf8')}, 'name': 'memes'.encode('utf8')},
				'description': 'Not really valuable mosaic'.encode('utf8'),
				'properties': [
					{'property_': {'name': 'divisibility'.encode('utf8'), 'value': '3'.encode('utf8')}},
					{'property_': {'name': 'initialSupply'.encode('utf8'), 'value': '123_000'.encode('utf8')}},
					{'property_': {'name': 'supplyMutable'.encode('utf8'), 'value': 'false'.encode('utf8')}},
					{'property_': {'name': 'transferable'.encode('utf8'), 'value': 'true'.encode('utf8')}}
				],
				'levy': {}
			}
		},

		# with levy
		{
			'type': 'mosaic_definition_transaction_v1',
			'rental_fee_sink': 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
			'rental_fee': 50000 * 1000000,

			'mosaic_definition': {
				'owner_public_key': '00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				'id': {'namespace_id': {'name': 'genes'.encode('utf8')}, 'name': 'memes'.encode('utf8')},
				'description': 'Not really valuable mosaic'.encode('utf8'),
				'properties': [
				],
				'levy': {
					'transfer_fee_type': 'absolute',
					'recipient_address': sample_address,
					'mosaic_id': {'namespace_id': {'name': 'lieutenant'.encode('utf8')}, 'name': 'colonel'.encode('utf8')},
					'fee': 632_0000
				}
			}
		},

		# supply change
		{
			'type': 'mosaic_supply_change_transaction_v1',
			'mosaic_id': {'namespace_id': {'name': 'genes'.encode('utf8')}, 'name': 'memes'.encode('utf8')},
			'action': 'increase',
			'delta': 321_000
		}
	]
