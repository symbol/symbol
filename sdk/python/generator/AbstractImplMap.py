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
            discriminator_name = parent_type.yaml_descriptor['discriminator']
            initializers = parent_type.yaml_descriptor['initializers']
            discriminator_value = [init['value'] for init in initializers if init['target_property_name'] == discriminator_name][0]
            self.mapping[parent_type.typename] = {
                'discriminator_name': discriminator_name,
                'discriminator_value': discriminator_value,
                'children': [],
            }

        self.mapping[parent_type.typename]['children'].append(child_type)

    def get(self, name):
        return self.mapping[name]
