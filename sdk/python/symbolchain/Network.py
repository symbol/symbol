from abc import abstractmethod

from symbolchain.ripemd160 import ripemd160

BASE32_RFC4648_ALPHABET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567'


class Network:
	"""Represents a network."""

	def __init__(self, name, identifier, datetime_converter, address_class, network_timestamp_class):
		# pylint: disable=too-many-arguments

		"""Creates a new network with the specified properties."""
		self.name = name
		self.identifier = identifier
		self.datetime_converter = datetime_converter

		self.address_class = address_class
		self.network_timestamp_class = network_timestamp_class

	def public_key_to_address(self, public_key):
		"""Converts a public key to an address."""
		part_one_hash_builder = self.address_hasher()
		part_one_hash_builder.update(public_key.bytes)
		part_one_hash = part_one_hash_builder.digest()

		part_two_hash = ripemd160(part_one_hash)

		version = bytes([self.identifier]) + part_two_hash

		part_three_hash_builder = self.address_hasher()
		part_three_hash_builder.update(version)
		checksum = part_three_hash_builder.digest()[0:4]

		return self.create_address(version, checksum)

	def is_valid_address_string(self, address_string):
		"""Checks if an address string is valid and belongs to this network."""
		if self.address_class.ENCODED_SIZE != len(address_string):
			return False

		if any(ch not in BASE32_RFC4648_ALPHABET for ch in address_string):
			return False

		return self.is_valid_address(self.address_class(address_string))

	def is_valid_address(self, address):
		"""Checks if an address is valid and belongs to this network."""
		if address.bytes[0] != self.identifier:
			return False

		hash_builder = self.address_hasher()
		hash_builder.update(address.bytes[0:1 + 20])

		checksum_from_address = address.bytes[1 + 20:]
		calculated_checksum = hash_builder.digest()[0:len(checksum_from_address)]
		return checksum_from_address == calculated_checksum

	def to_datetime(self, reference_network_timestamp):
		"""Converts a network timestamp to a datetime."""
		return self.datetime_converter.to_datetime(reference_network_timestamp.timestamp)

	def from_datetime(self, reference_datetime):
		"""Converts a datetime to a network timestamp."""
		return self.network_timestamp_class(self.datetime_converter.to_difference(reference_datetime))

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
