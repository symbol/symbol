from .CatsParseException import CatsParseException


class Scope:
    """Tuple composed of filename and line number"""
    def __init__(self, name):
        self.name = name
        self.line_number = 0


class ScopeManager:
    """Manages the current scope composed of filename and line number"""
    def __init__(self):
        self.scopes = [Scope('<unknown>')]

    def push_scope(self, name):
        """Pushes the input scope"""
        self.scopes.append(Scope(name))

    def scope(self):
        """Gets the current location"""
        return ['{0}:{1}'.format(scope.name, scope.line_number) for scope in self.scopes][::-1]

    def pop_scope(self):
        """Pops the input scope"""
        if 1 == len(self.scopes):
            raise CatsParseException('CatsParser cannot pop default scope')

        self.scopes.pop()

    def increment_line_number(self):
        """Increments the line number for the current scope"""
        self.scopes[-1].line_number += 1
