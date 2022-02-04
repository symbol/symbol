import importlib
import inspect
import os
from binascii import hexlify
from pathlib import Path

import yaml

from symbolchain.facade.NemFacade import NemFacade
from symbolchain.facade.SymbolFacade import SymbolFacade


def read_test_vectors_file(filepath):
	with open(filepath, 'rt', encoding='utf8') as input_file:
		return yaml.safe_load(input_file)


def get_modules(network_name):
	module_names = {
		'nem': ['account_key_link', 'cosignature', 'mosaic_definition', 'mosaic_supply_change', 'multisig',
			'multisig_account_modification', 'namespace_registration', 'transfer'],
		'symbol': ['account_address_restriction', 'account_metadata', 'account_mosaic_restriction', 'account_operation_restriction',
			'address_alias', 'aggregate_complete', 'aggregate_mosaic_supply_change', 'hash_lock', 'key_link', 'mosaic_address_restriction',
			'mosaic_alias', 'mosaic_definition', 'mosaic_global_restriction', 'mosaic_metadata', 'mosaic_supply_change',
			'multisig_account_modification', 'namespace_metadata', 'namespace_registration', 'secret_lock', 'secret_proof','transfer']
	}

	modules = []
	for module_name in module_names[network_name]:
		module = importlib.import_module(f'testvectors.{network_name}.{module_name}')
		modules.append((module_name, module))

	return modules


def prepare_transaction_test_cases(network_name):
	filepath = Path(os.environ.get('SCHEMAS_PATH', '.')) / network_name / 'transactions' / 'transactions.yaml'
	return read_test_vectors_file(filepath)


class VectorGenerator:
	def __init__(self, network_name):
		test_cases = prepare_transaction_test_cases(network_name)
		self.network_name = network_name
		self.modules = get_modules(network_name)
		self.cases = {case['test_name']: case for case in test_cases}
		facade_class = {'symbol': SymbolFacade, 'nem': NemFacade}[network_name]
		self.facade = facade_class('testnet')

	def verify(self, test_name, transaction):
		case = self.cases[test_name]

		new_payload = hexlify(transaction.serialize()).decode('utf8').upper()
		if case['payload'] != new_payload:
			print(transaction)

			print('original:', case['payload'])
			print('     new:', new_payload)
			raise RuntimeError('payload does not match')

	def generate(self):
		for module_descriptor in self.modules:
			generate = getattr(module_descriptor[1], 'generate')
			transaction_map = generate(self.facade.transaction_factory)
			for test_name, transaction in transaction_map.items():
				self.verify(test_name, transaction)
			print(f'[+] module {self.network_name}.{module_descriptor[0]}: ok')


def main():
	for network_name in ['symbol', 'nem']:
		generator = VectorGenerator(network_name)
		generator.generate()


if '__main__' == __name__:
	main()
