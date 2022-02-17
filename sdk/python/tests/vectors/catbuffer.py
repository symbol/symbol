import importlib
import os
import re
from binascii import hexlify, unhexlify
from pathlib import Path

import pytest
import yaml

# region common test utils


def prepare_test_cases(network_name, includes=None, excludes=None):
	cases = []
	schemas_path = Path(os.environ.get('SCHEMAS_PATH', '.')) / network_name / 'transactions'

	if not schemas_path.exists():
		raise Exception(f'could not find any cases because {schemas_path} does not exist')

	for filepath in schemas_path.glob('*.yaml'):
		if includes and not any(include in filepath.name for include in includes):
			print(f'skipping {filepath.name} due to include filters')
			continue

		if excludes and any(exclude in filepath.name for exclude in excludes):
			print(f'skipping {filepath.name} due to exclude filters')
			continue

		with open(filepath, 'rt', encoding='utf8') as infile:
			cases += yaml.safe_load(infile)

	if not cases:
		raise Exception(f'could not find any cases in {schemas_path}')

	return cases


def to_hex_string(binary):
	return hexlify(binary).decode('ascii').upper()


def generate_pretty_id(val):
	return val['test_name']


# endregion

# region create from descriptor

def fixup_descriptor_common(descriptor, module):
	hex_string_pattern = re.compile('^[0-9A-F]+$')

	for key, value in descriptor.items():
		# skip false positive due to ABC123 value that should be treated as plain string
		if 'value' == key and 'namespace_metadata_transaction' == descriptor.get('type'):
			continue

		if isinstance(value, str) and hex_string_pattern.match(value):
			descriptor[key] = unhexlify(value)
		elif isinstance(value, list):
			for value_item in value:
				if isinstance(value_item, dict):
					fixup_descriptor_common(value_item, module)
		elif isinstance(value, dict):
			fixup_descriptor_common(value, module)


def fixup_descriptor_nem(descriptor, module, facade):
	descriptor['signature'] = getattr(module, 'Signature')(descriptor['signature'])
	fixup_descriptor_common(descriptor, module)

	if 'inner_transaction' in descriptor:
		fixup_descriptor_nem(descriptor['inner_transaction'], module, facade)
		descriptor['inner_transaction'] = facade.transaction_factory.to_non_verifiable_transaction(
			facade.transaction_factory.create(descriptor['inner_transaction'])
		)

		if 'cosignatures' in descriptor:
			for cosignature in descriptor['cosignatures']:
				cosignature['cosignature']['signature'] = getattr(module, 'Signature')(cosignature['cosignature']['signature'])


def fixup_cosignature_symbol(descriptor, module):
	cosignature = getattr(module, 'Cosignature')()
	cosignature.signature = getattr(module, 'Signature')(descriptor['signature'])
	cosignature.signer_public_key = getattr(module, 'PublicKey')(descriptor['signer_public_key'])
	return cosignature


def fixup_descriptor_symbol(descriptor, module, facade):
	descriptor['signature'] = getattr(module, 'Signature')(descriptor['signature'])
	fixup_descriptor_common(descriptor, module)

	if 'transactions' in descriptor:
		descriptor['transactions'] = [
			facade.transaction_factory.create_embedded(child_descriptor) for child_descriptor in descriptor['transactions']
		]

		if 'cosignatures' in descriptor:
			descriptor['cosignatures'] = [
				fixup_cosignature_symbol(cosignature_descriptor, module) for cosignature_descriptor in descriptor['cosignatures']
			]


def is_key_in_formatted_string(transaction, key):
	if key in str(transaction):
		return True

	return 'parent_name' == key and getattr(transaction, key) is None


def assert_create_from_descriptor(item, module, facade_name, fixup_descriptor):
	# Arrange:
	comment = item.get('comment', '')
	payload_hex = item['payload']

	facade_module = importlib.import_module(f'symbolchain.facade.{facade_name}')
	facade_class = getattr(facade_module, facade_name)
	facade = facade_class('testnet')

	descriptor = item['descriptor']
	fixup_descriptor(descriptor, module, facade)

	# Act:
	transaction = facade.transaction_factory.create(descriptor)
	transaction_buffer = transaction.serialize()

	# Assert:
	assert payload_hex == to_hex_string(transaction_buffer), comment
	assert all(is_key_in_formatted_string(transaction, key) for key in descriptor.keys()), comment


@pytest.mark.parametrize('item', prepare_test_cases('nem'), ids=generate_pretty_id)
def test_create_from_descriptor_nem(item):
	assert_create_from_descriptor(item, importlib.import_module('symbolchain.nc'), 'NemFacade', fixup_descriptor_nem)


@pytest.mark.parametrize('item', prepare_test_cases('symbol', includes=['transactions']), ids=generate_pretty_id)
def test_create_from_descriptor_symbol(item):  # pylint: disable=invalid-name
	assert_create_from_descriptor(item, importlib.import_module('symbolchain.sc'), 'SymbolFacade', fixup_descriptor_symbol)


# endregion

# region roundtrip

def assert_roundtrip(item, module):
	# Arrange:
	schema_name = item['schema_name']
	comment = item.get('comment', '')
	payload_hex = item['payload']
	payload = unhexlify(payload_hex)

	transaction_class = getattr(module, schema_name)

	# Act:
	transaction = transaction_class.deserialize(payload)
	transaction_buffer = transaction.serialize()

	# Assert:
	assert payload_hex == to_hex_string(transaction_buffer), comment
	assert len(transaction_buffer) == transaction.size, comment

	if schema_name.endswith('Transaction'):
		assert_roundtrip({'schema_name': 'TransactionFactory', 'payload': payload_hex, 'comment': comment}, module)


@pytest.mark.parametrize('item', prepare_test_cases('nem'), ids=generate_pretty_id)
def test_roundtrip_nem(item):
	assert_roundtrip(item, importlib.import_module('symbolchain.nc'))


@pytest.mark.parametrize('item', prepare_test_cases('symbol', excludes=['invalid']), ids=generate_pretty_id)
def test_roundtrip_symbol(item):
	assert_roundtrip(item, importlib.import_module('symbolchain.sc'))

# endregion
