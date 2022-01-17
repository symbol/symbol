#!/usr/bin/env python

#
# Reads utf8 encoded files with names "part<id>.txt".
# Creates aggregate transaction where each files is a message inside embedded transfers.
# Signs aggregate transaction using private key provided in file specified via `--private` switch.
#

import argparse
from pathlib import Path

import sha3

from examples.examples_utils import read_contents, read_private_key
from symbolchain.CryptoTypes import Hash256
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.MerkleHashBuilder import MerkleHashBuilder


def add_embedded_transfers(facade, public_key):
	# obtain recipient from public_key, so direct all transfers to 'self'
	embedded_transactions = []
	recipient = facade.network.public_key_to_address(public_key)
	resources = Path(__file__).parent / 'resources'
	for filepath in sorted(resources.glob('part*.txt')):
		msg = read_contents(filepath)
		embedded = facade.transaction_factory.create_embedded({
			'type': 'transfer',
			'signer_public_key': public_key,
			'recipient_address': recipient,
			# note: additional 0 byte at the beginning is added for compatibility with explorer
			# and other tools that treat messages starting with 00 byte as "plain text"
			'message': bytes(1) + msg.encode('utf8')
		})

		embedded_transactions.append(embedded)

		print(f'----> {filepath.name} length in bytes: {len(embedded.message)}')

	return embedded_transactions


def main():
	parser = argparse.ArgumentParser(description='create aggregate')
	parser.add_argument('--private', help='path to file with private key', required=True)
	args = parser.parse_args()

	facade = SymbolFacade('testnet')
	key_pair = read_private_key(args.private)

	embedded_transactions = add_embedded_transfers(facade, key_pair.public_key)
	hash_builder = MerkleHashBuilder()
	for embedded_transaction in embedded_transactions:
		embedded_transaction_hash = sha3.sha3_256(embedded_transaction.serialize()).digest()
		hash_builder.update(Hash256(embedded_transaction_hash))

	merkle_hash = hash_builder.final()

	aggregate = facade.transaction_factory.create({
		'type': 'aggregate_complete',
		'signer_public_key': key_pair.public_key,
		'fee': 0,
		'deadline': 1,
		'transactions_hash': merkle_hash,
		'transactions': embedded_transactions
	})

	signature = facade.sign_transaction(key_pair, aggregate)
	aggregate.signature = signature.bytes

	print(aggregate)


if __name__ == '__main__':
	main()
