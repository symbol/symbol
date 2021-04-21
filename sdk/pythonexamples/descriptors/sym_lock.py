from binascii import unhexlify

from symbolchain.core.CryptoTypes import Hash256
from symbolchain.core.facade.SymFacade import SymFacade


def descriptor_factory():
    sample_address = SymFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')
    sample_mosaic_id = 0x7EDCBA90FEDCBA90
    secret = unhexlify('C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000')

    return [
        # note: only network currency can be used as a mosaic in hash lock
        {
            'type': 'hashLock',
            'mosaic': (sample_mosaic_id, 123_000000),
            'duration': 123,
            'hash': Hash256.zero()
        },

        {
            'type': 'secretLock',
            'mosaic': (sample_mosaic_id, 123_000000),
            'duration': 123,
            'recipient_address': sample_address,
            'secret': secret,
            'hash_algorithm': 'hash_160'
        },

        {
            'type': 'secretProof',
            'recipient_address': sample_address,
            'secret': secret,
            'hash_algorithm': 'hash_160',
            'proof': unhexlify('C1ECFDFC')
        }
    ]
