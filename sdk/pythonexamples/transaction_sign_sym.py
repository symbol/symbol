#!/usr/bin/env python

#
# Shows how to create all transactions manually using TransactionFactory.
#

from binascii import hexlify, unhexlify

from symbolchain.core.CryptoTypes import Hash256, PrivateKey, PublicKey
from symbolchain.core.facade.SymFacade import SymFacade
from symbolchain.core.sym.IdGenerator import generate_mosaic_id, generate_namespace_id

SAMPLE_MOSAIC_ID = 0x7EDCBA90FEDCBA90
SAMPLE_NAMESPACE_ID = 0xC01DFEE7FEEDDEAD
MESSAGE = 'V belom plashche s krovavym podboyem, sharkayushchey kavaleriyskoy pokhodkoy'


class SymTransactionSample:
    # pylint: disable=too-many-public-methods

    def __init__(self):
        self.facade = SymFacade('public_test')
        self.key_pair = self.facade.KeyPair(PrivateKey(unhexlify('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF')))
        self.sample_address = self.facade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y')
        self.sample_public_key = PublicKey(unhexlify('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F'))

    def run_all(self):
        transaction_descriptors = [
            self.account_address_restriction_1(),
            self.account_address_restriction_2(),
            self.account_mosaic_restriction(),
            self.account_operation_restriction(),

            self.address_alias(),
            self.mosaic_alias(),

            self.account_key_link(),
            self.node_key_link(),
            self.voting_key_link(),
            self.vrf_key_link(),

            self.hash_lock(),
            self.secret_lock(),
            self.secret_proof(),

            self.namespace_root_registration(),
            self.namespace_child_registration(),
            self.mosaic_definition(),
            self.mosaic_supply_change(),

            self.account_metadata(),
            self.mosaic_metadata(),
            self.namespace_metadata(),

            self.mosaic_global_restriction(),
            self.mosaic_address_restriction(),

            self.transfer_without_message(),
            self.transfer_without_mosaics(),
            self.transfer()
        ]

        for descriptor in transaction_descriptors:
            self.set_common_fields(descriptor)
            transaction = self.facade.transaction_factory.create(descriptor)
            self.sign_and_print(transaction)

    def set_common_fields(self, descriptor):
        common_fields = {
            'signer_public_key': self.key_pair.public_key.bytes,
            'fee': 625,
            'deadline': 12345
        }
        descriptor.update(common_fields)

    def sign_and_print(self, transaction):
        signature = self.facade.sign_transaction(self.key_pair, transaction)
        self.facade.transaction_factory.attach_signature(transaction, signature)

        print(transaction)
        print(hexlify(transaction.serialize()))
        print('---- '*20)

    # region account restrictions

    def account_address_restriction_1(self):
        # allow incoming transactions only from address below
        return {
            'type': 'accountAddressRestriction',
            'restriction_flags': 'address',
            'restriction_additions': [self.sample_address.bytes]
        }

    def account_address_restriction_2(self):
        # block transactions outgoing to given address
        # note: block and allow restrictions are mutually exclusive, documentation
        # https://docs.symbolplatform.com/concepts/account-restriction.html#account-restriction
        return {
            'type': 'accountAddressRestriction',
            'restriction_flags': 'address outgoing block',
            'restriction_additions': [self.sample_address.bytes]
        }

    @staticmethod
    def account_mosaic_restriction():
        allowed_mosaic_id = SAMPLE_MOSAIC_ID
        return {
            'type': 'accountMosaicRestriction',
            'restriction_flags': 'mosaic_id',
            'restriction_additions': [allowed_mosaic_id]
        }

    @staticmethod
    def account_operation_restriction():
        # allow only specific transaction types
        return {
            'type': 'accountOperationRestriction',
            'restriction_flags': 'outgoing',
            'restriction_additions': [
                'transfer_transaction',
                'account_key_link_transaction',
                'vrf_key_link_transaction',
                'voting_key_link_transaction',
                'node_key_link_transaction'
            ]
        }

    # endregion

    # region aliases

    def address_alias(self):
        return {
            'type': 'addressAlias',
            'namespace_id': SAMPLE_NAMESPACE_ID,
            'address': self.sample_address.bytes,
            'alias_action': 'link'
        }

    @staticmethod
    def mosaic_alias():
        return {
            'type': 'mosaicAlias',
            'namespace_id': SAMPLE_NAMESPACE_ID,
            'mosaic_id': SAMPLE_MOSAIC_ID,
            'alias_action': 'link'
        }

    # endregion

    # region key links

    def account_key_link(self):
        return {
            'type': 'accountKeyLink',
            'linked_public_key': self.sample_public_key.bytes,
            'link_action': 'link'
        }

    def node_key_link(self):
        return {
            'type': 'nodeKeyLink',
            'linked_public_key': self.sample_public_key.bytes,
            'link_action': 'link'
        }

    def voting_key_link(self):
        return {
            'type': 'votingKeyLink',
            'linked_public_key': self.sample_public_key.bytes,
            'link_action': 'link',
            'start_epoch': 10,
            'end_epoch': 150
        }

    def vrf_key_link(self):
        return {
            'type': 'vrfKeyLink',
            'linked_public_key': self.sample_public_key.bytes,
            'link_action': 'link'
        }

    # endregion

    # region locks

    @staticmethod
    def hash_lock():
        # note: only network currency can be used as a mosaic in hash lock
        return {
            'type': 'hashLock',
            'mosaic': (SAMPLE_MOSAIC_ID, 123_000000),
            'duration': 123,
            'hash': Hash256.zero().bytes
        }

    def secret_lock(self):
        return {
            'type': 'secretLock',
            'mosaic': (SAMPLE_MOSAIC_ID, 123_000000),
            'duration': 123,
            'recipient_address': self.sample_address.bytes,
            'secret': unhexlify('C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000'),
            'hash_algorithm': 'hash_160'
        }

    def secret_proof(self):
        return {
            'type': 'secretProof',
            'recipient_address': self.sample_address.bytes,
            'secret': unhexlify('C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000'),
            'hash_algorithm': 'hash_160',
            'proof': unhexlify('C1ECFDFC')
        }

    # endregion

    # region namespace and mosaic

    @staticmethod
    def namespace_root_registration():
        return {
            'type': 'namespaceRegistration',
            'registration_type': 'root',
            'duration': 123,
            'name': 'roger'
        }

    @staticmethod
    def namespace_child_registration():
        return {
            'type': 'namespaceRegistration',
            'registration_type': 'child',
            'parent_id': generate_namespace_id('roger'),
            'name': 'charlie'
        }

    @staticmethod
    def mosaic_definition():
        return {
            'type': 'mosaicDefinition',
            'duration': 1,
            'nonce': 123,
            'flags': 'transferable restrictable',
            'divisibility': 2
        }

    def mosaic_supply_change(self):
        address = self.facade.network.public_key_to_address(self.key_pair.public_key)
        return {
            'type': 'mosaicSupplyChange',
            'mosaic_id': generate_mosaic_id(address, 123),
            'delta': 1000 * 100,  # assuming divisibility = 2
            'action': 'increase'
        }

    # endregion

    # region metadata

    def account_metadata(self):
        value = 'much coffe, such wow'
        return {
            'type': 'accountMetadata',
            'target_address': self.sample_address.bytes,
            'scoped_metadata_key': 0xC0FFE,
            'value_size_delta': len(value),
            'value': value
        }

    def mosaic_metadata(self):
        value = 'Once upon a midnight dreary'
        return {
            'type': 'mosaicMetadata',
            'target_mosaic_id': SAMPLE_MOSAIC_ID,
            'target_address': self.sample_address.bytes,
            'scoped_metadata_key': 0xFACADE,
            'value_size_delta': len(value),
            'value': value
        }

    def namespace_metadata(self):
        value = 'while I pondered, weak and weary'
        return {
            'type': 'namespaceMetadata',
            'target_namespace_id': SAMPLE_NAMESPACE_ID,
            'target_address': self.sample_address.bytes,
            'scoped_metadata_key': 0xC1CADA,
            'value_size_delta': len(value),
            'value': value
        }

    # endregion

    # region mosaic restrictions

    @staticmethod
    def mosaic_global_restriction():
        return {
            'type': 'mosaicGlobalRestriction',
            'mosaic_id': SAMPLE_MOSAIC_ID,
            'reference_mosaic_id': 0,
            'restriction_key': 0x0A0D474E5089,
            'previous_restriction_value': 0,
            'new_restriction_value': 2,
            'previous_restriction_type': 0,
            'new_restriction_type': 'ge'
        }

    def mosaic_address_restriction(self):
        return {
            'type': 'mosaicAddressRestriction',
            'mosaic_id': SAMPLE_MOSAIC_ID,
            'restriction_key': 0x0A0D474E5089,
            'previous_restriction_value': 0,
            'new_restriction_value': 5,
            'target_address': self.sample_address.bytes
        }

    # endregion

    # region transfers

    def basic_transfer(self):
        return {
            'type': 'transfer',
            'recipient_address': self.sample_address.bytes
        }

    def transfer_without_message(self):
        return {
            **self.basic_transfer(),
            'mosaics': [
                (SAMPLE_MOSAIC_ID, 12345_000000)
            ]
        }

    def transfer_without_mosaics(self):
        return {
            **self.basic_transfer(),
            'message': MESSAGE
        }

    def transfer(self):
        return {
            **self.basic_transfer(),
            'message': MESSAGE,
            'mosaics': [
                (SAMPLE_MOSAIC_ID, 12345_000000),
                (0x1234567812345678, 10)
            ]
        }

    # endregion


def main():
    sample = SymTransactionSample()
    sample.run_all()


if __name__ == '__main__':
    main()
