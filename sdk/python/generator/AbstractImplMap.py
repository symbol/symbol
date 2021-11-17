from type_objects import StructObject


class AbstractImplMap:
    """
    This container is used to create map of all objects implementing some base objects.

    In case of transactions, Transaction and EmbeddedTransaction types are considered base objects.
    """

    def __init__(self):
        self.mapping = {}

    def add(self, parent_type: StructObject, child_type: StructObject):
        if parent_type.typename not in self.mapping:
            self.mapping[parent_type.typename] = {
                'discriminator_name': 'type_',
                'discriminator_value': 'TRANSACTION_TYPE',
                'children': [],
            }

        self.mapping[parent_type.typename]['children'].append(child_type)

    def get(self, name):
        return self.mapping[name]
