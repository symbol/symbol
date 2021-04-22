import argparse
import json
import os
import sys
import time
from binascii import unhexlify

from symbolchain.core.Bip32 import Bip32
from symbolchain.core.CryptoTypes import PrivateKey, PublicKey, Signature
from symbolchain.core.Network import NetworkLocator
from symbolchain.core.sym.IdGenerator import generate_mosaic_id


class ClassLocator:
    def __init__(self, facade_class, network_class):
        self.facade_class = facade_class
        self.key_pair_class = self.facade_class.KeyPair
        self.verifier_class = self.facade_class.Verifier
        self.network_class = network_class
        self.address_class = self.facade_class.Address

    @property
    def bip32_root_node_factory(self):
        return Bip32(self.facade_class.BIP32_CURVE_NAME)


class VectorsTestSuite:
    def __init__(self, identifier, filename, description):
        self.identifier = identifier
        self._filename = filename
        self.description = description

    @property
    def filename(self):
        return '{}.{}'.format(self.identifier, self._filename)


class KeyConversionTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(1, 'test-keys', 'key conversion')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        # Arrange:
        private_key = PrivateKey(test_vector['privateKey'])
        expected_public_key = PublicKey(test_vector['publicKey'])

        # Act:
        actual_public_key = self.class_locator.key_pair_class(private_key).public_key

        # Assert:
        return [(expected_public_key, actual_public_key)]


class AddressConversionTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(1, 'test-address', 'address conversion')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        # Arrange:
        public_key = PublicKey(test_vector['publicKey'])
        expected_address_mainnet = self.class_locator.address_class(test_vector['address_Public'])
        expected_address_testnet = self.class_locator.address_class(test_vector['address_PublicTest'])

        mainnet = NetworkLocator.find_by_name(self.class_locator.network_class.NETWORKS, ['public', 'mainnet'])
        testnet = NetworkLocator.find_by_name(self.class_locator.network_class.NETWORKS, ['public_test', 'testnet'])

        # Act:
        actual_address_mainnet = mainnet.public_key_to_address(public_key)
        actual_address_testnet = testnet.public_key_to_address(public_key)

        # Assert:
        return [
            (expected_address_mainnet, actual_address_mainnet),
            (expected_address_testnet, actual_address_testnet)
        ]


class SignTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(2, 'test-sign', 'sign')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        # Arrange:
        private_key = PrivateKey(test_vector['privateKey'])
        message = unhexlify(test_vector['data'])
        expected_signature = Signature(test_vector['signature'])

        # Act:
        actual_signature = self.class_locator.key_pair_class(private_key).sign(message)

        # Assert:
        return [(expected_signature, actual_signature)]


class VerifyTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(2, 'test-sign', 'verify')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        # Arrange:
        public_key = PublicKey(test_vector['publicKey'])
        message = unhexlify(test_vector['data'])
        signature = Signature(test_vector['signature'])

        # Act:
        is_verified = self.class_locator.verifier_class(public_key).verify(message, signature)

        # Assert:
        return [(True, is_verified)]


class MosaicIdDerivationTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(5, 'test-mosaic-id', 'mosaic id derivation')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        # Arrange:
        nonce = test_vector['mosaicNonce']

        mosaic_id_pairs = []
        for network_tag in ['Public', 'PublicTest', 'Private', 'PrivateTest']:
            address = self.class_locator.address_class(test_vector['address_' + network_tag])
            expected_mosaic_id = test_vector['mosaicId_' + network_tag]

            # Act:
            mosaic_id = generate_mosaic_id(address, nonce)

            # Assert:
            mosaic_id_pairs.append((expected_mosaic_id, '{:016X}'.format(mosaic_id)))

        return mosaic_id_pairs


class Bip32DerivationTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(6, 'test-hd-derivation', 'BIP32 derivation')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        # Arrange:
        seed = unhexlify(test_vector['seed'])
        expected_root_public_key = PublicKey(test_vector['rootPublicKey'])

        expected_child_public_keys = []
        for child_test_vector in test_vector['childAccounts']:
            expected_child_public_keys.append(PublicKey(child_test_vector['publicKey']))

        # Act:
        root_node = self.class_locator.bip32_root_node_factory.from_seed(seed)
        root_public_key = self.class_locator.key_pair_class(root_node.private_key).public_key

        child_public_keys = []
        for child_test_vector in test_vector['childAccounts']:
            child_node = root_node.derive_path(child_test_vector['path'])
            child_key_pair = self.class_locator.facade_class.bip32_node_to_key_pair(child_node)
            child_public_keys.append(child_key_pair.public_key)

        # Assert:
        return [
            (expected_root_public_key, root_public_key),
            (expected_child_public_keys, child_public_keys)
        ]


class Bip39DerivationTester(VectorsTestSuite):
    def __init__(self, class_locator):
        super().__init__(6, 'test-hd-derivation', 'BIP39 derivation')
        self.class_locator = class_locator

    def process(self, test_vector, _):
        if 'mnemonic' not in test_vector:
            return None

        # Arrange:
        mnemonic = test_vector['mnemonic']
        passphrase = test_vector['passphrase']
        expected_root_public_key = PublicKey(test_vector['rootPublicKey'])

        # Act:
        root_node = self.class_locator.bip32_root_node_factory.from_mnemonic(mnemonic, passphrase)
        root_public_key = self.class_locator.key_pair_class(root_node.private_key).public_key

        # Assert:
        return [(expected_root_public_key, root_public_key)]


def load_class_locator(blockchain):
    # pylint: disable=import-outside-toplevel

    if 'symbol' == blockchain:
        from symbolchain.core.facade.SymFacade import SymFacade
        from symbolchain.core.sym.Network import Network
        return ClassLocator(SymFacade, Network)

    from symbolchain.core.facade.NisFacade import NisFacade
    from symbolchain.core.nis1.Network import Network
    return ClassLocator(NisFacade, Network)


def main():
    # pylint: disable=too-many-locals

    test_identifiers = range(0, 7)
    parser = argparse.ArgumentParser(description='nem test vectors harness')
    parser.add_argument('--vectors', help='path to test-vectors directory', required=True)
    parser.add_argument('--blockchain', choices=['nis1', 'symbol'], default='symbol')
    parser.add_argument(
        '--tests',
        help='identifiers of tests to include',
        type=int,
        nargs='+',
        choices=test_identifiers,
        default=test_identifiers)
    args = parser.parse_args()

    class_locator = load_class_locator(args.blockchain)
    test_suites = [
        KeyConversionTester(class_locator),
        AddressConversionTester(class_locator),
        SignTester(class_locator),
        VerifyTester(class_locator),
        Bip32DerivationTester(class_locator),
        Bip39DerivationTester(class_locator),
    ]

    if 'symbol' == args.blockchain:
        test_suites += [MosaicIdDerivationTester(class_locator)]

    num_failed_suites = 0
    for test_suite in test_suites:
        if test_suite.identifier not in args.tests:
            print('[ SKIPPED ] {} test'.format(test_suite.description))
            continue

        with open(os.path.join(args.vectors, '{}.json'.format(test_suite.filename)), 'r') as infile:
            start_time = time.time()

            test_case_number = 0
            num_failed = 0

            parsed_json = json.loads(infile.read())
            for test_vectors in parsed_json:
                if isinstance(test_vectors, str):
                    test_group_name = test_vectors
                    test_vectors = parsed_json[test_group_name]
                else:
                    test_group_name = None
                    test_vectors = [test_vectors]

                for test_vector in test_vectors:
                    processed_pairs = test_suite.process(test_vector, test_group_name)
                    if None is processed_pairs:
                        continue

                    if any(pair[0] != pair[1] for pair in processed_pairs):
                        num_failed += 1

                    test_case_number += 1

            elapsed_time = time.time() - start_time

            test_message_prefix = '[{:8.4f}s] {} test:'.format(elapsed_time, test_suite.description)
            if num_failed:
                print('{} {} failures out of {}'.format(test_message_prefix, num_failed, test_case_number))
                num_failed_suites += 1
            else:
                print('{} successes {}'.format(test_message_prefix, test_case_number))

    if num_failed_suites:
        sys.exit(1)


if '__main__' == __name__:
    main()
