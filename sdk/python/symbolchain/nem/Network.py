import base64
import datetime

import sha3

from ..ByteArray import ByteArray
from ..Network import Network as BasicNetwork
from ..NetworkTimestamp import NetworkTimestamp as BasicNetworkTimestamp
from ..NetworkTimestamp import NetworkTimestampDatetimeConverter


class NetworkTimestamp(BasicNetworkTimestamp):
	"""Represents a nem network timestamp with second resolution."""

	def add_seconds(self, count):
		return NetworkTimestamp(self.timestamp + count)


class Address(ByteArray):
	"""Represents a nem address."""

	SIZE = 25
	ENCODED_SIZE = 40

	def __init__(self, address):
		"""Creates an address from a decoded or encoded address."""
		raw_bytes = address
		if isinstance(address, str):
			raw_bytes = base64.b32decode(address)
		elif isinstance(address, Address):
			raw_bytes = address.bytes

		super().__init__(self.SIZE, raw_bytes, Address)

	def __str__(self):
		return base64.b32encode(self.bytes).decode('utf8')

	def __repr__(self):
		return f'Address(\'{str(self)}\')'


class Network(BasicNetwork):
	"""Represents a nem network."""

	def __init__(self, name, identifier, epoch_time):
		"""Creates a new network with the specified properties."""
		super().__init__(name, identifier, NetworkTimestampDatetimeConverter(epoch_time, 'seconds'), Address, NetworkTimestamp)

	def address_hasher(self):
		return sha3.keccak_256()

	def create_address(self, address_without_checksum, checksum):
		return Address(address_without_checksum + checksum)


Network.MAINNET = Network('mainnet', 0x68, datetime.datetime(2015, 3, 29, 0, 6, 25, tzinfo=datetime.timezone.utc))
Network.TESTNET = Network('testnet', 0x98, datetime.datetime(2015, 3, 29, 0, 6, 25, tzinfo=datetime.timezone.utc))
Network.NETWORKS = [Network.MAINNET, Network.TESTNET]
