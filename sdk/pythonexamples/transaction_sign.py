#!/usr/bin/env python

#
# Shows how to create all transactions manually using TransactionFactory.
#

import argparse
import importlib
from abc import abstractmethod
from binascii import hexlify, unhexlify

from symbolchain.core.CryptoTypes import PrivateKey
from symbolchain.core.facade.NisFacade import NisFacade
from symbolchain.core.facade.SymFacade import SymFacade


class TransactionSample:
    def __init__(self, facade):
        self.facade = facade
        self.key_pair = self.facade.KeyPair(PrivateKey(unhexlify('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF')))

    def process_transaction_descriptors(self, transaction_descriptors):
        for descriptor in transaction_descriptors:
            self.set_common_fields(descriptor)
            transaction = self.facade.transaction_factory.create(descriptor)
            self.sign_and_print(transaction)

    @abstractmethod
    def set_common_fields(self, descriptor):
        pass

    def sign_and_print(self, transaction):
        signature = self.facade.sign_transaction(self.key_pair, transaction)
        self.facade.transaction_factory.attach_signature(transaction, signature)

        print(transaction)
        print(hexlify(transaction.serialize()))
        print('---- ' * 20)


class NisTransactionSample(TransactionSample):
    def __init__(self):
        super().__init__(NisFacade('testnet'))

    def set_common_fields(self, descriptor):
        descriptor.update({
            'signer_public_key': self.key_pair.public_key,
            'deadline': 12345
        })


class SymTransactionSample(TransactionSample):
    def __init__(self):
        super().__init__(SymFacade('public_test'))

    def set_common_fields(self, descriptor):
        descriptor.update({
            'signer_public_key': self.key_pair.public_key,
            'fee': 625,
            'deadline': 12345
        })


def main():
    parser = argparse.ArgumentParser(description='transaction sign example')
    parser.add_argument('--blockchain', help='blockchain', choices=('nis1', 'symbol'), required=True)
    args = parser.parse_args()

    if 'nis1' == args.blockchain:
        factory_names = [
            'descriptors.nis1_importance_transfer',
            'descriptors.nis1_transfer'
        ]
        sample = NisTransactionSample()
    else:
        factory_names = [
            'descriptors.sym_alias',
            'descriptors.sym_key_link',
            'descriptors.sym_lock',
            'descriptors.sym_metadata',
            'descriptors.sym_mosaic',
            'descriptors.sym_namespace',
            'descriptors.sym_restriction_account',
            'descriptors.sym_restriction_mosaic',
            'descriptors.sym_transfer'
        ]
        sample = SymTransactionSample()

    total_descriptors_count = 0
    for factory_name in factory_names:
        transaction_descriptor_factory = getattr(importlib.import_module(factory_name), 'descriptor_factory')
        transaction_descriptors = transaction_descriptor_factory()
        sample.process_transaction_descriptors(transaction_descriptors)
        total_descriptors_count += len(transaction_descriptors)

    print('finished processing {} descriptors'.format(total_descriptors_count))


if __name__ == '__main__':
    main()
