# NOTE: in nem, cosignature is a transaction

def descriptor_factory():
	sample_address = 'TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C'

	return [
		{
			'type': 'cosignature_transaction_v1',
			'multisig_transaction_hash': '677035391CD3701293D385F037BA32796252BB7CE180B00B582DD9B20AAAD7F0',
			'multisig_account_address': sample_address
		}
	]
