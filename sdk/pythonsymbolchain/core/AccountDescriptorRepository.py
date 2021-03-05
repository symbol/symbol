import yaml

from .CryptoTypes import PublicKey


class AccountDescriptor:
    """Represents an account."""

    # pylint: disable=too-few-public-methods

    def __init__(self, descriptor_yaml):
        """Creates a descriptor from a yaml container."""
        self.public_key = descriptor_yaml.get('public_key')
        if self.public_key:
            self.public_key = PublicKey(self.public_key)

        self.address = descriptor_yaml.get('address')
        self.name = descriptor_yaml.get('name')
        self.roles = descriptor_yaml.get('roles') or []


class AccountDescriptorRepository:
    """Loads read-only account descriptors from YAML."""

    def __init__(self, yaml_input):
        """Loads account descriptors from the specified input."""
        descriptors_yaml = yaml.load(yaml_input, Loader=yaml.SafeLoader)
        self.descriptors = [AccountDescriptor(descriptor_yaml) for descriptor_yaml in descriptors_yaml]

    def try_find_by_name(self, name):
        """Finds the account descriptor with a matching name or None if no matching descriptors are found."""
        return next((descriptor for descriptor in self.descriptors if name == descriptor.name), None)

    def find_by_public_key(self, public_key):
        """Finds the account descriptor with a matching public key."""
        return next(descriptor for descriptor in self.descriptors if descriptor.public_key and public_key == descriptor.public_key)

    def find_all_by_role(self, role):
        """Finds all account descriptors with a matching role."""
        return [descriptor for descriptor in self.descriptors if not role or role in descriptor.roles]
