SAMPLE_ADDRESS_ALIAS = 'TFIXOYLI2JBFPWAAAAAAAAAAAAAAAAAAAAAAAAA'
SAMPLE_ADDRESS = 'TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I'
SCHEMA_NAME = 'MosaicSupplyRevocationTransactionV1'


transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_supply_revocation_transaction_v1',
			'source_address': SAMPLE_ADDRESS,
			'mosaic': {'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
		}
	},
	# comment: alias address
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'mosaic_supply_revocation_transaction_v1',
			'source_address': SAMPLE_ADDRESS_ALIAS,
			'mosaic': {'mosaic_id': 0x000056CE00002B67, 'amount': 0x0000000000000064}
		}
	}
]
