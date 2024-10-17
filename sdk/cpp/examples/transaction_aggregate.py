#!/usr/bin/env python

#
# Reads utf8 encoded files with names "part<id>.txt".
# Creates aggregate transaction where each files is a message inside embedded transfers.
# Signs aggregate transaction using private key provided in file specified via `--private` switch.
#

import argparse
from pathlib import Path

from examples.examples_utils import read_contents, read_private_key
from symbolchain.facade.SymbolFacade import SymbolFacade


def add_embedded_transfers(facade, public_key):
	# obtain recipient from public_key, so direct all transfers to 'self'
	embedded_transactions = []
	recipient_address = facade.network.public_key_to_address(public_key)
	resources_directory = Path(__file__).parent / 'resources'
	for filepath in sorted(resources_directory.glob('part*.txt')):
		message = read_contents(filepath)
		embedded_transaction = facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction_v1',
			'signer_public_key': public_key,
			'recipient_address': recipient_address,
			# note: additional 0 byte at the beginning is added for compatibility with explorer
			# and other tools that treat messages starting with 00 byte as "plain text"
			'message': bytes(1) + message.encode('utf8')
		})

		embedded_transactions.append(embedded_transaction)

		print(f'----> {filepath.name} length in bytes: {len(embedded_transaction.message)}')

	return embedded_transactions


def main():
	parser = argparse.ArgumentParser(description='create aggregate')
	parser.add_argument('--private', help='path to file with private key', required=True)
	args = parser.parse_args()

	facade = SymbolFacade('testnet')
	key_pair = read_private_key(args.private)

	embedded_transactions = add_embedded_transfers(facade, key_pair.public_key)
	merkle_hash = facade.hash_embedded_transactions(embedded_transactions)

	aggregate_transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v2',
		'signer_public_key': key_pair.public_key,
		'fee': 0,
		'deadline': 1,
		'transactions_hash': merkle_hash,
		'transactions': embedded_transactions
	})

	signature = facade.sign_transaction(key_pair, aggregate_transaction)
	facade.transaction_factory.attach_signature(aggregate_transaction, signature)

	print(f'Hash: {facade.hash_transaction(aggregate_transaction)}\n')
	print(aggregate_transaction)


if __name__ == '__main__':
	main()
