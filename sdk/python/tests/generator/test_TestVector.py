import importlib
import os
from binascii import hexlify, unhexlify
from pathlib import Path

import pytest
import yaml


def read_test_vectors_file(filepath):
	with open(filepath, 'rt', encoding='utf8') as input_file:
		return yaml.safe_load(input_file)


def prepare_test_cases(network_name):
	cases = []
	schemas_path = Path(os.environ.get('SCHEMAS_PATH', '.')) / network_name

	if not schemas_path.exists():
		return cases

	for filepath in schemas_path.glob('*.yml'):
		cases += read_test_vectors_file(filepath)

	return cases


def to_hex_string(binary: bytes):
	return hexlify(binary).decode('ascii').upper()


def generate_pretty_id(val):
	if dict != type(val):
		return None

	return val['test_name']


def prepare_payload(payload):
	# some basevalue items in yaml are enclosed in qutoes
	return unhexlify(payload.replace('\'', ''))


@pytest.mark.parametrize('item', prepare_test_cases('symbol'), ids=generate_pretty_id)
def test_serialize_symbol(item):
	schema_name = item['schema_name']
	comment = item.get('comment', '')
	payload = item['payload']

	module = importlib.import_module('symbolchain.sc')

	if schema_name == 'Key':
		schema_name = 'PublicKey'

	builder_class = getattr(module, schema_name)
	builder = builder_class.deserialize(prepare_payload(item['payload']))

	serialized = builder.serialize()
	assert to_hex_string(serialized) == payload.upper(), comment

	if schema_name.endswith('Transaction'):
		generic_builder_class = getattr(module, 'TransactionFactory')
		factory_builder = generic_builder_class.deserialize(prepare_payload(item['payload']))
		factory_serialized = factory_builder.serialize()

		assert to_hex_string(factory_serialized) == payload.upper(), comment


@pytest.mark.parametrize('item', prepare_test_cases('nem'), ids=generate_pretty_id)
def test_serialize_nem(item):
	schema_name = item['schema_name']
	comment = item.get('comment', '')
	payload = item['payload']

	module = importlib.import_module('symbolchain.nc')

	builder_class = getattr(module, schema_name)
	builder = builder_class.deserialize(prepare_payload(item['payload']))

	serialized = builder.serialize()
	assert to_hex_string(serialized) == payload.upper(), comment

	if schema_name.endswith('Transaction'):
		generic_builder_class = getattr(module, 'TransactionFactory')
		factory_builder = generic_builder_class.deserialize(prepare_payload(item['payload']))
		factory_serialized = factory_builder.serialize()

		assert to_hex_string(factory_serialized) == payload.upper(), comment

