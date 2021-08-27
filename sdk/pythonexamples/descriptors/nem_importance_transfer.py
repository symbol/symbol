from symbolchain.core.CryptoTypes import PublicKey


def descriptor_factory():
    return [
        {
            'type': 'importance-transfer',
            'mode': 1,
            'remote_account_public_key': PublicKey('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F')
        }
    ]
