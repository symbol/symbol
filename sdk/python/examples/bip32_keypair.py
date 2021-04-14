#!/usr/bin/env python

#
# Shows how to use Bip32 interface to derive keys.
#
# note: this example is *not* generating keys comaptible with wallet
#

from symbolchain.core.Bip32 import Bip32
from symbolchain.core.facade.SymFacade import SymFacade


def derive_key(root_node, facade, change, index):
    path = [44, facade.BIP32_COIN_ID, 0, change, index]

    child_node = root_node.derive_path(path)
    child_key_pair = facade.bip32_node_to_key_pair(child_node)

    print(' PATH: {}'.format(path))
    print(' * private key: {}'.format(child_key_pair.private_key))
    print(' *  public key: {}'.format(child_key_pair.public_key))
    address = facade.network.public_key_to_address(child_key_pair.public_key)
    print(' *     address: {}'.format(address))
    print()


def main():
    facade = SymFacade('public_test')

    bip = Bip32(facade.BIP32_CURVE_NAME)
    root_node = bip.from_mnemonic('axis buzz cycle dynamic eyebrow future gym hybrid ivory just know lyrics', 'correcthorsebatterystaple')

    for change in [0, 1]:
        for index in range(3):
            derive_key(root_node, facade, change, index)


if __name__ == '__main__':
    main()
