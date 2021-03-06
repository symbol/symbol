import hashlib
from abc import abstractmethod


class Network:
    """Represents a network."""

    def __init__(self, name, identifier):
        """Creates a new network with the specified name and identifier byte."""
        self.name = name
        self.identifier = identifier

    def public_key_to_address(self, public_key):
        """Converts a public key to an address."""
        part_one_hash_builder = self.address_hasher()
        part_one_hash_builder.update(public_key.bytes)
        part_one_hash = part_one_hash_builder.digest()

        part_two_hash_builder = hashlib.new('ripemd160')
        part_two_hash_builder.update(part_one_hash)
        part_two_hash = part_two_hash_builder.digest()

        version = bytes([self.identifier]) + part_two_hash

        part_three_hash_builder = self.address_hasher()
        part_three_hash_builder.update(version)
        checksum = part_three_hash_builder.digest()[0:4]

        return self.create_address(version, checksum)

    @abstractmethod
    def address_hasher(self):
        """Gets the primary hasher to use in the public key to address conversion."""

    @abstractmethod
    def create_address(self, address_without_checksum, checksum):
        """Creates an encoded address from an address without checksum and checksum bytes."""

    def __eq__(self, other):
        return isinstance(other, Network) and self.name == other.name and self.identifier == other.identifier

    def __str__(self):
        return self.name


class NetworkLocator:
    """Provides utility functions for finding a network."""

    @staticmethod
    def find_by_name(networks, names):
        """Finds a network with a specified name within a list of networks."""
        if isinstance(names, str):
            names = [names]

        return next(network for network in networks if network.name in names)

    @staticmethod
    def find_by_identifier(networks, identifiers):
        """Finds a network with a specified identifier within a list of networks."""
        if isinstance(identifiers, int):
            identifiers = [identifiers]

        return next(network for network in networks if network.identifier in identifiers)
