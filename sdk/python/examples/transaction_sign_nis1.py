#!/usr/bin/env python

#
# Shows how to create all transactions manually using TransactionFactory.
#

from binascii import hexlify, unhexlify

from symbolchain.core.CryptoTypes import PrivateKey, PublicKey
from symbolchain.core.facade.NisFacade import NisFacade

MESSAGE = 'V belom plashche s krovavym podboyem, sharkayushchey kavaleriyskoy pokhodkoy'


class NisTransactionSample:
    def __init__(self):
        self.facade = NisFacade('testnet')
        self.key_pair = self.facade.KeyPair(PrivateKey(unhexlify('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF')))
        self.sample_address = self.facade.Address('TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C')
        self.sample_public_key = PublicKey(unhexlify('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'))

    def run_all(self):
        transaction_descriptors = [
            self.importance_transfer(),

            self.transfer({'amount': 12345_000000}),
            self.transfer({'message': MESSAGE}),
            self.transfer({
                'amount': 12345_000000,
                'message': MESSAGE
            })
        ]

        for descriptor in transaction_descriptors:  # pylint: disable=duplicate-code
            self.set_common_fields(descriptor)
            transaction = self.facade.transaction_factory.create(descriptor)
            self.sign_and_print(transaction)

    def set_common_fields(self, descriptor):
        # pylint: disable=duplicate-code
        common_fields = {
            'signer_public_key': self.key_pair.public_key,
            'deadline': 12345
        }
        descriptor.update(common_fields)

    def sign_and_print(self, transaction):
        # pylint: disable=duplicate-code
        signature = self.facade.sign_transaction(self.key_pair, transaction)
        self.facade.transaction_factory.attach_signature(transaction, signature)

        print(transaction)
        print(hexlify(transaction.serialize()))
        print('---- '*20)

    # region importance transfer

    def importance_transfer(self):
        return {
            'type': 'importance-transfer',
            'mode': 1,
            'remote_account_public_key': self.sample_public_key
        }

    # endregion

    # region transfers

    def transfer(self, additional_properties):
        return {
            'type': 'transfer',
            'recipient_address': self.sample_address,
            **additional_properties
        }

    # endregion


def main():
    sample = NisTransactionSample()
    sample.run_all()


if __name__ == '__main__':
    main()
