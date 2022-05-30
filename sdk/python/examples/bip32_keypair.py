#!/usr/bin/env python

#
# Shows how to use Bip32 interface to derive keys.
#
# note: this example is *not* generating keys compatible with wallet
#

from symbolchain.Bip32 import Bip32
from symbolchain.facade.SymbolFacade import SymbolFacade


def derive_key(root_node, facade, change, index):
	path = [*facade.bip32_path(0)[:-2], change, index]

	child_node = root_node.derive_path(path)
	child_key_pair = facade.bip32_node_to_key_pair(child_node)

	print(f' PATH: {path}')
	print(f' * private key: {child_key_pair.private_key}')
	print(f' *  public key: {child_key_pair.public_key}')

	address = facade.network.public_key_to_address(child_key_pair.public_key)
	print(f' *     address: {address}')
	print()


def main():
	facade = SymbolFacade('testnet')

	bip = Bip32(facade.BIP32_CURVE_NAME)
	root_node = bip.from_mnemonic(
		'cat swing flag economy stadium alone churn speed unique patch report train',
		'correcthorsebatterystaple')

	for change in [0, 1]:
		for index in range(3):
			derive_key(root_node, facade, change, index)


if __name__ == '__main__':
	main()
