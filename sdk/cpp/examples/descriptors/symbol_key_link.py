def descriptor_factory():
	sample_public_key = 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'

	return [
		{
			'type': 'account_key_link_transaction_v1',
			'linked_public_key': sample_public_key,
			'link_action': 'link'
		},

		{
			'type': 'node_key_link_transaction_v1',
			'linked_public_key': sample_public_key,
			'link_action': 'link'
		},

		{
			'type': 'voting_key_link_transaction_v1',
			'linked_public_key': sample_public_key,
			'link_action': 'link',
			'start_epoch': 10,
			'end_epoch': 150
		},

		{
			'type': 'vrf_key_link_transaction_v1',
			'linked_public_key': sample_public_key,
			'link_action': 'link'
		}
	]
