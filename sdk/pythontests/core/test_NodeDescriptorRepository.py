import unittest

import yaml

from symbolchain.core.NodeDescriptorRepository import NodeDescriptorRepository

YAML_INPUT = '''
- host: ALICE
  roles: [historical, XXL]

- host: bob
  roles: [backup, super]

- host: charlie

- host: mercury
  roles: [super]
'''


class NodeDescriptorRepositoryTest(unittest.TestCase):
    def test_can_load_descriptors_yaml(self):
        # Arrange:
        repository = NodeDescriptorRepository(YAML_INPUT)

        # Assert:
        self.assertEqual(4, len(repository.descriptors))
        self.assertEqual(['ALICE', 'bob', 'charlie', 'mercury'], [descriptor.host for descriptor in repository.descriptors])

    def test_can_load_descriptors_list(self):
        # Arrange:
        repository = NodeDescriptorRepository(yaml.load(YAML_INPUT, Loader=yaml.SafeLoader))

        # Assert:
        self.assertEqual(4, len(repository.descriptors))
        self.assertEqual(['ALICE', 'bob', 'charlie', 'mercury'], [descriptor.host for descriptor in repository.descriptors])

    def _assert_can_find_all_by_role(self, role, expected_match_hosts):
        # Arrange:
        repository = NodeDescriptorRepository(YAML_INPUT)

        # Act:
        descriptors = repository.find_all_by_role(role)
        match_hosts = [descriptor.host for descriptor in descriptors]

        # Assert:
        self.assertEqual(expected_match_hosts, match_hosts)

    def test_find_all_by_role_when_no_match(self):
        self._assert_can_find_all_by_role('regular', [])

    def test_find_all_by_role_when_single_match(self):
        self._assert_can_find_all_by_role('historical', ['ALICE'])

    def test_find_all_by_role_when_multiple_matches(self):
        self._assert_can_find_all_by_role('super', ['bob', 'mercury'])

    def test_find_all_by_role_when_no_role_filter_is_provided(self):
        self._assert_can_find_all_by_role(None, ['ALICE', 'bob', 'charlie', 'mercury'])

    def _assert_can_find_all_not_by_role(self, role, expected_match_hosts):
        # Arrange:
        repository = NodeDescriptorRepository(YAML_INPUT)

        # Act:
        descriptors = repository.find_all_not_by_role(role)
        match_hosts = [descriptor.host for descriptor in descriptors]

        # Assert:
        self.assertEqual(expected_match_hosts, match_hosts)

    def test_find_all_not_by_role_when_no_match(self):
        self._assert_can_find_all_not_by_role('regular', ['ALICE', 'bob', 'charlie', 'mercury'])

    def test_find_all_not_by_role_when_single_match(self):
        self._assert_can_find_all_not_by_role('historical', ['bob', 'charlie', 'mercury'])

    def test_find_all_not_by_role_when_multiple_matches(self):
        self._assert_can_find_all_not_by_role('super', ['ALICE', 'charlie'])

    def test_find_all_not_by_role_when_no_role_filter_is_provided(self):
        self._assert_can_find_all_not_by_role(None, [])
