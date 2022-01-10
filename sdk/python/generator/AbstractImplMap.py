from .type_objects import StructObject


class AbstractImplMap:
    """
    This container is used to create map of all objects implementing some base objects.

    In case of transactions, Transaction and EmbeddedTransaction types are considered base objects.
    """

    def __init__(self):
        self.mapping = {}

    def add(self, parent_type: StructObject, child_type: StructObject):
        if parent_type.typename not in self.mapping:
            discriminator_names = parent_type.yaml_descriptor['discriminator']
            initializers = parent_type.yaml_descriptor['initializers']

            # need loop to maintain proper order
            discriminator_values = []
            for discriminator_name in discriminator_names:
                initializer_value = [init['value'] for init in initializers if discriminator_name == init['target_property_name']][0]
                discriminator_values.append(initializer_value)
            self.mapping[parent_type.typename] = {
                'discriminator_names': discriminator_names,
                'discriminator_values': discriminator_values,
                'children': [],
            }

        self.mapping[parent_type.typename]['children'].append(child_type)

    def get(self, name):
        return self.mapping[name]
