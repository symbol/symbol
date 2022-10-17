from binascii import unhexlify

from symbolchain.CryptoTypes import Hash256


def descriptor_factory():
	sample_address = 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y'
	sample_mosaic_id = 0x7EDCBA90FEDCBA90
	secret = 'C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000'

	return [
		# note: only network currency can be used as a mosaic in hash lock
		{
			'type': 'hash_lock_transaction_v1',
			'mosaic': {'mosaic_id': sample_mosaic_id, 'amount': 123_000000},
			'duration': 123,
			'hash': Hash256.zero()
		},

		{
			'type': 'secret_lock_transaction_v1',
			'mosaic': {'mosaic_id': sample_mosaic_id, 'amount': 123_000000},
			'duration': 123,
			'recipient_address': sample_address,
			'secret': secret,
			'hash_algorithm': 'hash_160'
		},

		{
			'type': 'secret_proof_transaction_v1',
			'recipient_address': sample_address,
			'secret': secret,
			'hash_algorithm': 'hash_160',
			'proof': unhexlify('C1ECFDFC')
		}
	]
