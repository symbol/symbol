#!/usr/bin/env python

#
# Shows how to create multisig account.
#

from binascii import unhexlify

import symbolchain.sc
from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.KeyPair import KeyPair


class MultisigAccountModificationSample:
	def __init__(self):
		self.facade = SymbolFacade('testnet')
		self.multisig_key_pair = KeyPair(PrivateKey(unhexlify('11002233445566778899AABBCCDDEEFF11002233445566778899AABBCCDDEEFF')))
		self.cosignatory_key_pairs = [
			KeyPair(PrivateKey(unhexlify('AABBCCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899'))),
			KeyPair(PrivateKey(unhexlify('BBCCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899AA'))),
			KeyPair(PrivateKey(unhexlify('CCDDEEFF11002233445566778899AABBCCDDEEFF11002233445566778899AABB')))
		]

	def run(self):
		# note: it's important to SIGN the transaction BEFORE adding cosignatures
		aggregate_transaction = self.create_aggregate_transaction()

		signature = self.facade.sign_transaction(self.multisig_key_pair, aggregate_transaction)
		self.facade.transaction_factory.attach_signature(aggregate_transaction, signature)

		self.add_cosignatures(aggregate_transaction)

		print(aggregate_transaction)

	def create_aggregate_transaction(self):
		embedded_transactions = [
			self.facade.transaction_factory.create_embedded({
				'type': 'multisig_account_modification_transaction',
				'signer_public_key': self.multisig_key_pair.public_key,
				'min_approval_delta': 1,
				'min_removal_delta': 1,
				'address_additions': list(map(self.to_address, self.cosignatory_key_pairs))
			})
		]

		return self.facade.transaction_factory.create({
			'type': 'aggregate_complete_transaction',
			'signer_public_key': self.multisig_key_pair.public_key,
			'fee': 625,
			'deadline': 12345,
			'transactions_hash': self.facade.hash_embedded_transactions(embedded_transactions).bytes,
			'transactions': embedded_transactions,
		})

	def to_address(self, key_pair):
		return self.facade.network.public_key_to_address(key_pair.public_key)

	def add_cosignatures(self, aggregate_transaction):
		transaction_hash = self.facade.hash_transaction(aggregate_transaction).bytes
		for key_pair in self.cosignatory_key_pairs:
			# version = 0
			cosignature = symbolchain.sc.Cosignature()
			cosignature.version = 0
			cosignature.signer_public_key = key_pair.public_key
			cosignature.signature = key_pair.sign(transaction_hash)
			aggregate_transaction.cosignatures.append(cosignature)


def main():
	sample = MultisigAccountModificationSample()
	sample.run()


if __name__ == '__main__':
	main()
