import os

import yaml

from ..CryptoTypes import PublicKey


class BatchOperations:
	"""Provides facade-based batch operations."""

	class PrepareError(Exception):
		"""Error that is thrown when a batch prepare operation fails."""

		def __init__(self, message):
			"""Creates a new error."""
			Exception.__init__(self, message)

	def __init__(self, facade, output_file_prefix=''):
		"""Creates batch operations around the specified facade."""
		self.facade = facade
		self.output_file_prefix = output_file_prefix

	def load_all(self, transactions_yaml_input):
		"""Loads all transactions from YAML."""
		return [
			self.facade.transaction_factory.create(transaction_descriptor)
			for transaction_descriptor in yaml.load(transactions_yaml_input, Loader=yaml.SafeLoader)
		]

	def _sign_one(self, transaction, private_key_storage, signature_storage, output_filename):
		# transaction.signer_public_key is of different PublicKey type, wrap it in sdk type
		signer_public_key = PublicKey(transaction.signer_public_key.bytes)
		signer_account_name = self.facade.account_descriptor_repository.find_by_public_key(signer_public_key).name
		signer_private_key = private_key_storage.load(signer_account_name)

		signature = self.facade.sign_transaction(self.facade.KeyPair(signer_private_key), transaction)
		transaction_hash = self.facade.hash_transaction(transaction)
		signature_storage.save(output_filename, transaction_hash, [signature])

	def sign_all(self, transactions, private_key_storage, signature_storage):
		"""Signs multiple transactions and saves the signatures as multiple qrcodes."""
		for i, transaction in enumerate(transactions):
			self._sign_one(transaction, private_key_storage, signature_storage, f'sig_{self.output_file_prefix}{i}')

	def prepare_all(self, transactions, signature_storage, output_directory):
		"""Prepares multiple transactions by attaching signatures to each and producing files that can be sent to the network."""
		transaction_count = len(transactions)
		signature_groups = [signature_storage.load(f'sig_{self.output_file_prefix}{i}') for i in range(0, transaction_count)]

		for i, transaction in enumerate(transactions):
			(signed_transaction_hash, signatures) = signature_groups[i]

			transaction_hash = self.facade.hash_transaction(transaction)
			if signed_transaction_hash != transaction_hash:
				raise self.PrepareError(f'transaction hash at {i} does not match signed transaction hash')

			if not self.facade.verify_transaction(transaction, signatures[0]):
				raise self.PrepareError(f'transaction signature at {i} does not verify')

		for i, transaction in enumerate(transactions):
			(_, signatures) = signature_groups[i]

			prepared_transaction_buffer = self.facade.transaction_factory.attach_signature(transaction, signatures[0])
			file_path = os.path.join(output_directory, f'payload_{self.output_file_prefix}{i}.json')
			with open(file_path, 'wb') as outfile:
				outfile.write(prepared_transaction_buffer)
