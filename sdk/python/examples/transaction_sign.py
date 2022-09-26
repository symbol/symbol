#!/usr/bin/env python

#
# Shows how to create all transactions manually using TransactionFactory.
#

import argparse
import importlib
from abc import abstractmethod
from binascii import hexlify, unhexlify

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.NemFacade import NemFacade
from symbolchain.facade.SymbolFacade import SymbolFacade


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

		print(f'Hash: {self.facade.hash_transaction(transaction)}')
		print(transaction)
		print(hexlify(transaction.serialize()))
		print('---- ' * 20)


class NemTransactionSample(TransactionSample):
	def __init__(self):
		super().__init__(NemFacade('testnet'))

	def set_common_fields(self, descriptor):
		descriptor.update({
			'signer_public_key': self.key_pair.public_key,
			'deadline': 12345
		})


class SymbolTransactionSample(TransactionSample):
	def __init__(self):
		super().__init__(SymbolFacade('testnet'))

	def set_common_fields(self, descriptor):
		descriptor.update({
			'signer_public_key': self.key_pair.public_key,
			'fee': 625,
			'deadline': 12345
		})


def main():
	parser = argparse.ArgumentParser(description='transaction sign example')
	parser.add_argument('--blockchain', help='blockchain', choices=('nem', 'symbol'), required=True)
	args = parser.parse_args()

	if 'nem' == args.blockchain:
		factory_names = [
			'nem_account_key_link',
			# 'nem_cosignature',
			'nem_mosaic',
			'nem_multisig_account',
			'nem_namespace',
			'nem_transfer'
		]
		sample = NemTransactionSample()
	else:
		factory_names = [
			'symbol_alias',
			'symbol_key_link',
			'symbol_lock',
			'symbol_metadata',
			'symbol_mosaic',
			'symbol_namespace',
			'symbol_restriction_account',
			'symbol_restriction_mosaic',
			'symbol_transfer'
		]
		sample = SymbolTransactionSample()

	total_descriptors_count = 0
	for factory_name in factory_names:
		transaction_descriptor_factory = getattr(importlib.import_module(f'examples.descriptors.{factory_name}'), 'descriptor_factory')
		transaction_descriptors = transaction_descriptor_factory()
		sample.process_transaction_descriptors(transaction_descriptors)
		total_descriptors_count += len(transaction_descriptors)

	print(f'finished processing {total_descriptors_count} descriptors')


if __name__ == '__main__':
	main()
