import sha3

from symbolchain.CryptoTypes import Hash256
from symbolchain.symbol.MerkleHashBuilder import MerkleHashBuilder


def calculate_transactions_hash(transactions):
	hash_builder = MerkleHashBuilder()
	for embedded_transaction in transactions:
		hash_builder.update(Hash256(sha3.sha3_256(embedded_transaction.serialize()).digest()))
	return hash_builder.final()
