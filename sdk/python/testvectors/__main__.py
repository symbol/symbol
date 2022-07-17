import argparse
import copy
import importlib
import json
from binascii import hexlify
from pathlib import Path

import sha3

from symbolchain import nc, sc
from symbolchain.ByteArray import ByteArray
from symbolchain.facade.NemFacade import NemFacade
from symbolchain.facade.SymbolFacade import SymbolFacade


def get_modules(network_name):
	module_path = Path(__file__).parent / network_name
	module_names = [file_path.stem for file_path in module_path.glob('*.py') if file_path.stem != '__init__']

	modules = []
	for module_name in sorted(module_names):
		module = importlib.import_module(f'testvectors.{network_name}.{module_name}')
		modules.append((module_name, module))

	return modules


def to_hex_string(buffer):
	return hexlify(buffer).decode('utf8').upper()


def clone_descriptor(descriptor):
	# note, this makes shallow clone
	cloned_descriptor = {}
	for key, value in descriptor.items():
		if key in ['deadline', 'signature', 'fee']:
			continue

		cloned_descriptor[key] = value

	return cloned_descriptor


class SymbolHelper:
	AGGREGATE_SCHEMA_NAME = 'AggregateBondedTransaction'
	Signature = sc.Signature

	def __init__(self, network_name):
		self.facade = SymbolFacade(network_name)

	def set_common_fields(self, descriptor, test_name):
		descriptor['signer_public_key'] = sha3.sha3_256(test_name.encode('utf8')).hexdigest()
		descriptor['signature'] = self.Signature(sha3.sha3_512(test_name.encode('utf8')).hexdigest())
		descriptor['fee'] = 0xFEEFEEFEEFEEFEE0
		descriptor['deadline'] = 0x71E71E71E71E71E0

	def create(self, test_name, original_descriptor):
		descriptor = clone_descriptor(original_descriptor)
		self.set_common_fields(descriptor, test_name)
		return self.facade.transaction_factory.create(descriptor), descriptor

	def create_aggregate_from_single(self, test_name, single_descriptor):
		return self.create_aggregate(test_name, {
			'aggregate': {'type': 'aggregate_bonded_transaction'},
			'embedded': [single_descriptor]
		})

	@staticmethod
	def create_cosignature_descriptor(test_name, index):
		name = f'{test_name}_cosig_{index+1}'
		return {
			'signer_public_key': sha3.sha3_256(name.encode('utf8')).hexdigest(),
			'signature': sha3.sha3_512(name.encode('utf8')).hexdigest()
		}

	@staticmethod
	def create_cosignature(descriptor):
		cosignature = sc.Cosignature()
		cosignature.signer_public_key = sc.PublicKey(descriptor['signer_public_key'])
		cosignature.signature = sc.Signature(descriptor['signature'])
		return cosignature

	def create_aggregate(self, test_name, original_descriptor):
		factory = self.facade.transaction_factory
		embedded_transactions = list(map(clone_descriptor, original_descriptor['embedded']))

		printable_descriptor = clone_descriptor(original_descriptor['aggregate'])
		printable_descriptor['transactions'] = embedded_transactions
		self.set_common_fields(printable_descriptor, test_name)
		printable_descriptor['cosignatures'] = [
			self.create_cosignature_descriptor(test_name, i) for i in range(original_descriptor.get('num_cosignatures', 0))
		]

		descriptor = copy.copy(printable_descriptor)
		descriptor['transactions'] = list(map(factory.create_embedded, embedded_transactions))
		descriptor['transactions_hash'] = SymbolFacade.hash_embedded_transactions(descriptor['transactions'])
		descriptor['cosignatures'] = list(map(self.create_cosignature, printable_descriptor['cosignatures']))

		printable_descriptor['transactions_hash'] = descriptor['transactions_hash']

		return factory.create(descriptor), printable_descriptor


class NemHelper:
	AGGREGATE_SCHEMA_NAME = 'MultisigTransaction'
	Signature = nc.Signature

	def __init__(self, network_name):
		self.facade = NemFacade(network_name)

	def set_common_fields(self, descriptor, test_name):
		descriptor['signer_public_key'] = sha3.sha3_256(test_name.encode('utf8')).hexdigest()
		descriptor['signature'] = self.Signature(sha3.sha3_512(test_name.encode('utf8')).hexdigest())
		descriptor['fee'] = 0xFEEFEEFEEFEEFEE0
		descriptor['timestamp'] = 0x71E71E70

	def create(self, test_name, original_descriptor):
		descriptor = clone_descriptor(original_descriptor)
		self.set_common_fields(descriptor, test_name)
		return self.facade.transaction_factory.create(descriptor), descriptor

	def create_aggregate_from_single(self, test_name, single_descriptor):
		return self.create_aggregate(test_name, {
			'aggregate': {'type': 'multisig_transaction'},
			'embedded': single_descriptor
		})

	def create_aggregate(self, test_name, original_descriptor):
		factory = self.facade.transaction_factory
		inner_descriptor = clone_descriptor(original_descriptor['embedded'])
		self.set_common_fields(inner_descriptor, test_name)

		printable_descriptor = clone_descriptor(original_descriptor['aggregate'])
		printable_descriptor['inner_transaction'] = inner_descriptor
		self.set_common_fields(printable_descriptor, test_name)
		printable_descriptor['cosignatures'] = [
			self.create_cosignature(test_name, i) for i in range(original_descriptor.get('num_cosignatures', 0))
		]

		descriptor = copy.copy(printable_descriptor)
		descriptor['inner_transaction'] = factory.to_non_verifiable_transaction(factory.create(inner_descriptor))
		return self.facade.transaction_factory.create(descriptor), printable_descriptor

	def create_cosignature(self, test_name, index):
		name = f'{test_name}_cosig_{index+1}'
		descriptor = {
			# note: `type: cosignature`` is not present, it's handled by TransactionDescriptorProcessor
			'multisig_transaction_hash': sha3.sha3_256(test_name.encode('utf8')).hexdigest(),
			'multisig_account_address': 'TBT7GACQQLYXUFBSQCUHXXWQMSRDAJPACTNJ724W'
		}
		self.set_common_fields(descriptor, name)

		return {'cosignature': descriptor}


class VectorGenerator:
	def __init__(self, network_name):
		self.network_name = network_name
		self.modules = get_modules(self.network_name)
		helper_class = {'symbol': SymbolHelper, 'nem': NemHelper}[self.network_name]
		self.helper = helper_class('testnet')

	def fix_descriptor_before_storing(self, descriptor):
		fixed = {}
		if not isinstance(descriptor, dict):
			return descriptor

		for key, value in descriptor.items():
			if isinstance(value, ByteArray):
				fixed[key] = str(value)
			elif isinstance(value, bytes):
				fixed[key] = to_hex_string(value)
			elif isinstance(value, dict):
				fixed[key] = self.fix_descriptor_before_storing(value)
			elif isinstance(value, list):
				fixed[key] = [self.fix_descriptor_before_storing(element) for element in value]
			else:
				fixed[key] = value
		return fixed

	def create_entry(self, schema_name, test_name, factory, descriptor):
		transaction, final_descriptor = factory(test_name, descriptor)
		return {
			'schema_name': schema_name,
			'test_name': test_name,
			'payload': to_hex_string(transaction.serialize()),
			'descriptor': self.fix_descriptor_before_storing(final_descriptor)
		}

	def create_transactions(self, module_descriptor, recipes):
		test_cases = []
		schema_name = recipes['schema_name']
		test_prefix = f'{schema_name}_{module_descriptor[0]}'
		for index, descriptor in enumerate(recipes['descriptors']):
			test_name = f'{test_prefix}_single_{index+1}'
			test_cases.append(self.create_entry(schema_name, test_name, self.helper.create, descriptor))

		# only thing _not_ supporting wrapping in aggregates is NEM's Cosignature (transaction)
		if recipes.get('single_only', False):
			return test_cases

		for index, descriptor in enumerate(recipes['descriptors']):
			test_name = f'{test_prefix}_aggregate_{index+1}'
			aggregate_schema_name = self.helper.AGGREGATE_SCHEMA_NAME
			test_cases.append(self.create_entry(aggregate_schema_name, test_name, self.helper.create_aggregate_from_single, descriptor))

		return test_cases

	def create_aggregate_transactions(self, module_descriptor, recipes):
		test_cases = []
		schema_name = recipes['schema_name']
		test_prefix = f'{schema_name}_{module_descriptor[0]}'

		for index, descriptor in enumerate(recipes['descriptors']):
			test_name = f'{test_prefix}_aggregate_{index+1}'
			test_cases.append(self.create_entry(schema_name, test_name, self.helper.create_aggregate, descriptor))

		return test_cases

	def generate(self):
		entries = []
		for module_descriptor in self.modules:
			if hasattr(module_descriptor[1], 'aggregate_recipes'):
				recipes = getattr(module_descriptor[1], 'aggregate_recipes')
				entries.extend(self.create_aggregate_transactions(module_descriptor, recipes))

			if hasattr(module_descriptor[1], 'recipes'):
				recipes = getattr(module_descriptor[1], 'recipes')
				entries.extend(self.create_transactions(module_descriptor, recipes))

			print(f'[+] module {self.network_name}.{module_descriptor[0]}: ok')

		return entries


def main():
	parser = argparse.ArgumentParser(
		description='generates transaction test vectors',
		prog=None if globals().get('__spec__') is None else f'python3 -m {__spec__.name.partition(".")[0]}'
	)
	parser.add_argument('--output', help='output directory', required=True)
	args = parser.parse_args()

	for network_name in ['symbol', 'nem']:
		generator = VectorGenerator(network_name)
		entries = generator.generate()

		filepath = Path(args.output) / network_name / 'models' / 'transactions.json'
		filepath.parent.mkdir(parents=True, exist_ok=True)

		with open(filepath, 'wt', encoding='utf8') as outfile:
			json.dump(entries, outfile, indent=2)


if '__main__' == __name__:
	main()
