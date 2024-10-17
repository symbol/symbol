SCHEMA_NAME = 'MultisigAccountModificationTransactionV1'

transactions = [  # pylint: disable=duplicate-code
	{
		'schema_name': SCHEMA_NAME,
		'descriptor': {
			'type': 'multisig_account_modification_transaction_v1',
			'min_removal_delta': 0x1,
			'min_approval_delta': 0x2,
			'address_additions': ['TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ', 'TD2ASJ2LKL5LX66PPZ67PYQN4HIMH5SX7OCZLQI'],
			'address_deletions': ['TCIFSMQZAX3IDPHUP2RTXP26N6BJRNKEBBKP33I']
		}
	}
]
