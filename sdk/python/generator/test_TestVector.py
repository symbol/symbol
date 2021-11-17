import importlib
import os
from binascii import hexlify, unhexlify
from collections import defaultdict
from pathlib import Path

import pytest
import yaml


def read_test_vectors_file(filepath):
    with open(filepath, 'rt', encoding='utf8') as input_file:
        return yaml.safe_load(input_file)


def prepare_test_cases():
    cases = []
    base_path = os.environ.get('HOME', '.')
    for filepath in Path(base_path + '/catbuffer-schemas/test-vectors/symbol').glob('*.yml'):
        cases += read_test_vectors_file(filepath)
    return cases


def to_hex_string(binary: bytes):
    return hexlify(binary).decode('ascii').upper()


def generate_pretty_id(val):
    generate_pretty_id.ids[val['builder']] += 1
    test_id = generate_pretty_id.ids[val['builder']]
    return f'{val["builder"]}_{test_id}'


generate_pretty_id.ids = defaultdict(int)  # type: ignore


def prepare_payload(payload):
    # some basevalue items in yaml are enclosed in qutoes
    return unhexlify(payload.replace('\'', ''))


@pytest.mark.parametrize('item', prepare_test_cases(), ids=generate_pretty_id)
def test_serialize(item):
    builder_name = item['builder']
    comment = item.get('comment', '')
    payload = item['payload']

    builder_module = importlib.import_module('symbolchain.sc')

    if builder_name == 'Key':
        builder_name = 'PublicKey'

    builder_class = getattr(builder_module, builder_name)
    builder = builder_class.deserialize(prepare_payload(item['payload']))

    serialized = builder.serialize()
    assert to_hex_string(serialized) == payload.upper(), comment
