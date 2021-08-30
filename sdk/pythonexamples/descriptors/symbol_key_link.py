from symbolchain.core.CryptoTypes import PublicKey


def descriptor_factory():
    sample_public_key = PublicKey('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F')

    return [
        {
            'type': 'accountKeyLink',
            'linked_public_key': sample_public_key,
            'link_action': 'link'
        },

        {
            'type': 'nodeKeyLink',
            'linked_public_key': sample_public_key,
            'link_action': 'link'
        },

        {
            'type': 'votingKeyLink',
            'linked_public_key': sample_public_key,
            'link_action': 'link',
            'start_epoch': 10,
            'end_epoch': 150
        },

        {
            'type': 'vrfKeyLink',
            'linked_public_key': sample_public_key,
            'link_action': 'link'
        }
    ]
