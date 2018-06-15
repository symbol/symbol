class CompositeTypeParser:
    """Base for composite type parsers"""
    def __init__(self, regex, factories):
        self.regex = regex
        self.sub_factories = factories
        self.type_name = None
        self.type_descriptor = None

    def factories(self):
        """Gets sub-parsers for this composite type parser"""
        return self.sub_factories

    def commit(self):
        """Returns the composite type tuple"""
        return (self.type_name, self.type_descriptor)
