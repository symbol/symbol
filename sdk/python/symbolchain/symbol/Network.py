import base64
import datetime
import hashlib

from ..ByteArray import ByteArray
from ..CryptoTypes import Hash256
from ..Network import Network as BasicNetwork
from ..NetworkTimestamp import NetworkTimestamp as BasicNetworkTimestamp
from ..NetworkTimestamp import NetworkTimestampDatetimeConverter


class NetworkTimestamp(BasicNetworkTimestamp):
	"""Represents a symbol network timestamp with millisecond resolution."""

	def add_milliseconds(self, count):
		"""Adds a specified number of milliseconds to this timestamp."""
		return NetworkTimestamp(self.timestamp + count)

	def add_seconds(self, count):
		return self.add_milliseconds(1000 * count)


class Address(ByteArray):
	"""Represents a Symbol address."""

	SIZE = 24
	ENCODED_SIZE = 39

	def __init__(self, address):
		"""Creates an address from a decoded or encoded address."""
		raw_bytes = address
		if isinstance(address, str):
			raw_bytes = base64.b32decode(address + 'A')[0:-1]
		elif isinstance(address, Address):
			raw_bytes = address.bytes

		super().__init__(self.SIZE, raw_bytes, Address)

	def __str__(self):
		return base64.b32encode(self.bytes + bytes(0)).decode('utf8')[0:-1]

	def __repr__(self):
		return f'Address(\'{str(self)}\')'


class Network(BasicNetwork):
	"""Represents a Symbol network."""

	def __init__(self, name, identifier, epoch_time, generation_hash_seed=None):
		"""Creates a new network with the specified properties."""
		super().__init__(name, identifier, NetworkTimestampDatetimeConverter(epoch_time, 'milliseconds'), Address, NetworkTimestamp)
		self.generation_hash_seed = generation_hash_seed

	def address_hasher(self):
		return hashlib.sha3_256()

	def create_address(self, address_without_checksum, checksum):
		return Address(address_without_checksum + checksum[0:3])


Network.MAINNET = Network(
	'mainnet',
	0x68,
	datetime.datetime(2021, 3, 16, 0, 6, 25, tzinfo=datetime.timezone.utc),
	Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6'))
Network.TESTNET = Network(
	'testnet',
	0x98,
	datetime.datetime(2022, 10, 31, 21, 7, 47, tzinfo=datetime.timezone.utc),
	Hash256('49D6E1CE276A85B70EAFE52349AACCA389302E7A9754BCF1221E79494FC665A4'))
Network.NETWORKS = [Network.MAINNET, Network.TESTNET]
