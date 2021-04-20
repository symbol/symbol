import os
import unittest
from binascii import hexlify, unhexlify
from collections import namedtuple

import yaml

from symbolchain.core.BufferReader import BufferReader
from symbolchain.core.CryptoTypes import PrivateKey, PublicKey, Signature
from symbolchain.core.sym.KeyPair import KeyPair, Verifier
from symbolchain.core.sym.VotingKeysGenerator import VotingKeysGenerator


class VotingKeysGeneratorTest(unittest.TestCase):
    # region private key generators

    class SeededPrivateKeyGenerator:
        def __init__(self, values):
            self.values = values
            self.next_index = 0

        def generate(self):
            self.next_index += 1
            return self.values[self.next_index - 1]

    class FibPrivateKeyGenerator:
        def __init__(self, fill_private_key=False):
            self.fill_private_key = fill_private_key
            self.value1 = 1
            self.value2 = 2

        def generate(self):
            next_value = self.value1 + self.value2
            self.value1 = self.value2
            self.value2 = next_value

            seed_value = next_value % 256

            if not self.fill_private_key:
                return PrivateKey(seed_value.to_bytes(PrivateKey.SIZE, 'big'))

            return PrivateKey(bytes([(seed_value + i) % 256 for i in range(0, PrivateKey.SIZE)]))

    # endregion

    # region basic test

    def test_can_generate_header(self):
        # Arrange:
        root_key_pair = KeyPair(PrivateKey.random())
        voting_keys_generator = VotingKeysGenerator(root_key_pair)

        # Act:
        voting_keys_buffer = voting_keys_generator.generate(7, 11)

        # Assert:
        self.assertEqual(32 + PublicKey.SIZE + 16 + 5 * (PrivateKey.SIZE + Signature.SIZE), len(voting_keys_buffer))

        reader = BufferReader(voting_keys_buffer)
        self.assertEqual(7, reader.read_int(8))
        self.assertEqual(11, reader.read_int(8))
        self.assertEqual(0xFFFFFFFFFFFFFFFF, reader.read_int(8))
        self.assertEqual(0xFFFFFFFFFFFFFFFF, reader.read_int(8))

        self.assertEqual(root_key_pair.public_key, PublicKey(reader.read_bytes(PublicKey.SIZE)))
        self.assertEqual(7, reader.read_int(8))
        self.assertEqual(11, reader.read_int(8))

    def test_can_generate_random_child_keys(self):
        # Arrange:
        root_key_pair = KeyPair(PrivateKey.random())
        voting_keys_generator = VotingKeysGenerator(root_key_pair)

        # Act:
        voting_keys_buffer = voting_keys_generator.generate(7, 11)

        # Assert:
        self.assertEqual(32 + PublicKey.SIZE + 16 + 5 * (PrivateKey.SIZE + Signature.SIZE), len(voting_keys_buffer))

        reader = BufferReader(voting_keys_buffer)
        reader.read_bytes(32 + PublicKey.SIZE + 16)  # skip header

        verifier = Verifier(root_key_pair.public_key)
        for i in reversed(range(0, 5)):
            child_private_key = PrivateKey(reader.read_bytes(PrivateKey.SIZE))
            signature = Signature(reader.read_bytes(Signature.SIZE))

            child_key_pair = KeyPair(child_private_key)
            signed_payload = child_key_pair.public_key.bytes + (7 + i).to_bytes(8, 'little')

            self.assertTrue(verifier.verify(signed_payload, signature), 'child at {}'.format(i))

    # endregion

    # region test vectors

    @staticmethod
    def _print_side_by_side_buffers_diff(expected_buffer, actual_buffer, batch_size):
        for i in range(0, len(expected_buffer) // batch_size):
            part_start_offset = i * batch_size
            part_end_offset = (i + 1) * batch_size
            expected_part = hexlify(expected_buffer[part_start_offset:part_end_offset]).decode('utf8').upper()
            actual_part = hexlify(actual_buffer[part_start_offset:part_end_offset]).decode('utf8').upper()
            print('E {} {} A {}'.format(expected_part, '==' if expected_part == actual_part else '!=', actual_part))

    def _run_test_vector(self, name, private_key_generator):
        # Arrange:
        test_vectors_filename = os.path.join(os.path.dirname(__file__), 'resources', 'voting_keys_generator_test_vectors.yaml')
        with open(test_vectors_filename, 'rt') as infile:
            test_vectors_yaml = yaml.load(infile.read(), Loader=yaml.SafeLoader)
            test_vector_yaml = next(test_vector_yaml for test_vector_yaml in test_vectors_yaml if name == test_vector_yaml['name'])
            test_vector = namedtuple('TestVector', test_vector_yaml.keys())(*test_vector_yaml.values())

            root_private_key = PrivateKey(test_vector.root_private_key)
            voting_keys_generator = VotingKeysGenerator(KeyPair(root_private_key), private_key_generator)

            # Act:
            voting_keys_buffer = voting_keys_generator.generate(test_vector.start_epoch, test_vector.end_epoch)

            # Assert:
            expected_voting_keys_buffer = unhexlify(test_vector.expected_file_hex)

            if expected_voting_keys_buffer != voting_keys_buffer:
                self._print_side_by_side_buffers_diff(expected_voting_keys_buffer, voting_keys_buffer, 16)

            self.assertEqual(expected_voting_keys_buffer, voting_keys_buffer)

    def test_can_generate_test_vector_1(self):
        self._run_test_vector('test_vector_1', self.FibPrivateKeyGenerator().generate)

    def test_can_generate_test_vector_2(self):
        self._run_test_vector('test_vector_2', self.FibPrivateKeyGenerator(True).generate)

    def test_can_generate_test_vector_3(self):
        # Arrange:
        private_key_generator = self.SeededPrivateKeyGenerator([
            PrivateKey('12F98B7CB64A6D840931A2B624FB1EACAFA2C25C3EF0018CD67E8D470A248B2F'),
            PrivateKey('B5593870940F28DAEE262B26367B69143AD85E43048D23E624F4ED8008C0427F'),
            PrivateKey('6CFC879ABCCA78F5A4C9739852C7C643AEC3990E93BF4C6F685EB58224B16A59')
        ])

        # Act + Assert:
        self._run_test_vector('test_vector_3', private_key_generator.generate)

    # endregion
