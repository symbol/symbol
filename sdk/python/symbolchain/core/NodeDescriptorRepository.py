import yaml


class NodeDescriptor:
    """Represents a node."""

    def __init__(self, descriptor_yaml):
        """Creates a descriptor from a yaml container."""
        self.host = descriptor_yaml.get('host')
        self.roles = descriptor_yaml.get('roles') or []


class NodeDescriptorRepository:
    """Loads read-only node descriptors from YAML."""

    def __init__(self, yaml_input):
        """Loads node descriptors from the specified input."""
        descriptors_yaml = yaml_input if isinstance(yaml_input, list) else yaml.load(yaml_input, Loader=yaml.SafeLoader)
        self.descriptors = [NodeDescriptor(descriptor_yaml) for descriptor_yaml in descriptors_yaml]

    def find_all_by_role(self, role):
        """Finds all node descriptors with a matching role."""
        return [descriptor for descriptor in self.descriptors if not role or role in descriptor.roles]

    def find_all_not_by_role(self, role):
        """Finds all node descriptors without a matching role."""
        return [descriptor for descriptor in self.descriptors if role and role not in descriptor.roles]
