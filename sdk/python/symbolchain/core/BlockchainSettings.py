import yaml

from .AccountDescriptorRepository import AccountDescriptorRepository
from .NodeDescriptorRepository import NodeDescriptorRepository


class BlockchainSettings:
    """Settings describing a blockchain."""

    def __init__(self, settings_dict):
        """Creates blockchain settings from a dictionary."""
        self.blockchain = settings_dict['blockchain']
        self.network = settings_dict['network']
        self.nodes = NodeDescriptorRepository(settings_dict['nodes'])
        self.accounts = AccountDescriptorRepository(settings_dict['accounts'])

    @staticmethod
    def load_from_yaml(yaml_input):
        """Loads settings from YAML."""
        settings_yaml = yaml.load(yaml_input, Loader=yaml.SafeLoader)
        return BlockchainSettings(settings_yaml)
