import yaml

from .CryptoTypes import PublicKey


class AccountDescriptor:
    """Represents an account."""

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
        descriptors_yaml = yaml_input if isinstance(yaml_input, list) else yaml.load(yaml_input, Loader=yaml.SafeLoader)
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

    def _lookup_account_descriptor_field(self, value, property_name, target_class):
        account_descriptor = self.try_find_by_name(value)
        if account_descriptor and hasattr(account_descriptor, property_name):
            return target_class(getattr(account_descriptor, property_name))

        return target_class(value)

    def _bind_lookup_account_descriptor_field(self, property_name, target_class):
        return lambda value: self._lookup_account_descriptor_field(value, property_name, target_class)

    def to_type_parsing_rules_map(self, type_to_property_mapping):
        """Builds a type to parsing rule map."""
        return {
            target_class: self._bind_lookup_account_descriptor_field(property_name, target_class)
            for target_class, property_name in type_to_property_mapping.items()
        }
