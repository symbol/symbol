import unittest

from catbuffer_parser.CatsParseException import CatsParseException
from catbuffer_parser.ScopeManager import ScopeManager


class ScopeManagerTest(unittest.TestCase):
    def test_manager_initially_has_default_scope(self):
        # Arrange:
        manager = ScopeManager()

        # Act:
        scope = manager.scope()

        # Assert:
        self.assertEqual(['<unknown>:0'], scope)

    def test_can_increment_line_number(self):
        # Arrange:
        manager = ScopeManager()

        # Act:
        manager.increment_line_number()
        manager.increment_line_number()
        manager.increment_line_number()
        scope = manager.scope()

        # Assert:
        self.assertEqual(['<unknown>:3'], scope)

    @staticmethod
    def _initialize_manager_with_three_scopes():
        # Arrange:
        manager = ScopeManager()

        # Act:
        manager.push_scope('zeta')
        manager.increment_line_number()
        manager.push_scope('beta')
        manager.increment_line_number()
        manager.increment_line_number()
        manager.push_scope('gamma')
        return manager

    def test_can_push_scopes(self):
        # Arrange:
        manager = self._initialize_manager_with_three_scopes()

        # Act:
        scope = manager.scope()

        # Assert:
        self.assertEqual(['gamma:0', 'beta:2', 'zeta:1', '<unknown>:0'], scope)

    def test_can_pop_some_scopes(self):
        # Arrange:
        manager = self._initialize_manager_with_three_scopes()

        # Act:
        manager.pop_scope()
        manager.pop_scope()
        scope = manager.scope()

        # Assert:
        self.assertEqual(['zeta:1', '<unknown>:0'], scope)

    def test_can_pop_all_scopes(self):
        # Arrange:
        manager = self._initialize_manager_with_three_scopes()

        # Act:
        manager.pop_scope()
        manager.pop_scope()
        manager.pop_scope()
        scope = manager.scope()

        # Assert:
        self.assertEqual(['<unknown>:0'], scope)

    def test_cannot_pop_default_scope(self):
        # Arrange:
        manager = ScopeManager()
        manager.push_scope('zeta')
        manager.pop_scope()

        # Act + Assert
        with self.assertRaises(CatsParseException):
            manager.pop_scope()
