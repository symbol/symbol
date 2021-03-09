#!/usr/bin/env python

#
# Shows how to create transaction manually using TransactionFactory and how to create transaction signature.
#

from binascii import hexlify, unhexlify

from symbolchain.core.CryptoTypes import PrivateKey
from symbolchain.core.facade.SymFacade import SymFacade
from symbolchain.core.sym.KeyPair import KeyPair
from symbolchain.core.sym.Network import Address


def main():
    facade = SymFacade('public_test')
    key_pair = KeyPair(PrivateKey(unhexlify('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF')))
    transaction = facade.transaction_factory.create({
        'type': 'transfer',
        'signerPublicKey': key_pair.public_key.bytes,
        'recipientAddress': Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y').bytes,
        'fee': 625,
        'deadline': 12345,
        'message': 'V belom plashche s krovavym podboyem, sharkayushchey kavaleriyskoy pokhodkoy,'.encode('utf-8'),
        'mosaics': [
            (0xFEDCBA90FEDCBA90, 12345_000000),
            (0x1234567812345678, 10)
        ]
    })

    signature = facade.sign_transaction(key_pair, transaction)
    facade.transaction_factory.attach_signature(transaction, signature)

    print(transaction)
    print(hexlify(transaction.serialize()))


if __name__ == '__main__':
    main()
