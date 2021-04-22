class TransactionDescriptorProcessor:
    """Processes and looks up transaction descriptor properties."""

    def __init__(self, transaction_descriptor, type_parsing_rules, type_converter=None):
        """Creates a transaction descriptor processor."""
        self.transaction_descriptor = transaction_descriptor
        self.type_parsing_rules = type_parsing_rules or {}
        self.type_converter = type_converter or (lambda x: x)
        self.type_hints = {}

    def _lookup_value_and_apply_type_hints(self, key):
        if key not in self.transaction_descriptor:
            raise ValueError('transaction descriptor does not have attribute {}'.format(key))

        value = self.transaction_descriptor[key]

        type_hint = self.type_hints.get(key)
        if type_hint in self.type_parsing_rules:
            value = self.type_parsing_rules[type_hint](value)

        return value

    def lookup_value(self, key):
        """Looks up the value for key."""
        value = self._lookup_value_and_apply_type_hints(key)
        if not any(isinstance(value, type_class) for type_class in [str, bytes, tuple]) and hasattr(value, '__iter__'):
            return [self.type_converter(item) for item in value]

        return self.type_converter(value)

    def copy_to(self, transaction, ignore_keys=None):
        """Copies all descriptor information to a transaction."""
        for key in self.transaction_descriptor.keys():
            if ignore_keys and key in ignore_keys:
                continue

            if not hasattr(transaction, key):
                raise ValueError('transaction does not have attribute {}'.format(key))

            value = self.lookup_value(key)
            if isinstance(value, list):
                getattr(transaction, key).extend(value)
            else:
                setattr(transaction, key, value)

    def set_type_hints(self, type_hints):
        """Sets type hints."""
        self.type_hints = type_hints or {}
