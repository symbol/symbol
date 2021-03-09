from ..CryptoTypes import PublicKey
from .Network import Address


class TypeParsingRules:
    """Rules for parsing nis types."""

    # pylint: disable=too-few-public-methods

    def __init__(self, account_descriptor_repository):
        """Creates parsing rules."""
        self.account_descriptor_repository = account_descriptor_repository

    def _lookup_account_descriptor_field(self, value, property_name, target_class):
        account_descriptor = self.account_descriptor_repository.try_find_by_name(value)
        if account_descriptor and getattr(account_descriptor, property_name):
            return target_class(getattr(account_descriptor, property_name))

        return target_class(value)

    def as_map(self):
        """Builds a type to parsing rule map."""
        return {
            Address: lambda value: self._lookup_account_descriptor_field(value, 'address', Address),
            PublicKey: lambda value: self._lookup_account_descriptor_field(value, 'public_key', PublicKey),
        }
