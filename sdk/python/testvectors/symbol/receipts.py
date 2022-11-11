from symbolchain.symbol.IdGenerator import generate_namespace_id

SAMPLE_ADDRESS = 'TBA6LOHEA6A465G2X5MSQF66JBYR254GJDPK7MQ'
SAMPLE_ADDRESS_2 = 'TD2ASJ2LKL5LX66PPZ67PYQN4HIMH5SX7OCZLQI'


receipts = [
	{
		'schema_name': 'HarvestFeeReceipt',
		'descriptor': {
			'type': 'harvest_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'InflationReceipt',
		'descriptor': {
			'type': 'inflation_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
		}
	},
	{
		'schema_name': 'LockHashCreatedFeeReceipt',
		'descriptor': {
			'type': 'lock_hash_created_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'LockHashCompletedFeeReceipt',
		'descriptor': {
			'type': 'lock_hash_completed_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'LockHashExpiredFeeReceipt',
		'descriptor': {
			'type': 'lock_hash_expired_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'LockSecretCreatedFeeReceipt',
		'descriptor': {
			'type': 'lock_secret_created_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'LockSecretCompletedFeeReceipt',
		'descriptor': {
			'type': 'lock_secret_completed_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'LockSecretExpiredFeeReceipt',
		'descriptor': {
			'type': 'lock_secret_expired_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'target_address': SAMPLE_ADDRESS
		}
	},
	{
		'schema_name': 'MosaicExpiredReceipt',
		'descriptor': {
			'type': 'mosaic_expired_receipt',
			'artifact_id': 0x85BBEA6CC462B244
		}
	},
	{
		'schema_name': 'MosaicRentalFeeReceipt',
		'descriptor': {
			'type': 'mosaic_rental_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'sender_address': SAMPLE_ADDRESS,
			'recipient_address': SAMPLE_ADDRESS_2
		}
	},
	{
		'schema_name': 'NamespaceExpiredReceipt',
		'descriptor': {
			'type': 'namespace_expired_receipt',
			'artifact_id': generate_namespace_id('evolving')
		}
	},
	{
		'schema_name': 'NamespaceExpiredReceipt',
		'descriptor': {
			'type': 'namespace_expired_receipt',
			'artifact_id': generate_namespace_id('evolving')
		}
	},
	{
		'schema_name': 'NamespaceRentalFeeReceipt',
		'descriptor': {
			'type': 'mosaic_rental_fee_receipt',
			'mosaic': {'mosaic_id': 0x85BBEA6CC462B244, 'amount': 0x1234567890ABCDEF},
			'sender_address': SAMPLE_ADDRESS,
			'recipient_address': SAMPLE_ADDRESS_2
		}
	},
]
