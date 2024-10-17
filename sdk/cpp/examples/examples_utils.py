from binascii import unhexlify
from pathlib import Path

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.PrivateKeyStorage import PrivateKeyStorage
from symbolchain.symbol.KeyPair import KeyPair


def read_contents(filepath) -> str:
	with open(filepath, 'rt', encoding='utf8') as infile:
		return infile.read()


def read_private_key(filepath, password=None) -> KeyPair:
	if not isinstance(filepath, Path):
		filepath = Path(filepath)

	if filepath.suffix == '.txt':
		return KeyPair(PrivateKey(unhexlify(read_contents(filepath).strip())))

	storage = PrivateKeyStorage(filepath.parent, password)
	return KeyPair(storage.load(filepath.stem))
