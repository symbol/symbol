from .ast import AstException, Struct, StructInlinePlaceholder


class AstPostProcessor:
    """Post processes AST type descriptors."""

    def __init__(self, type_descriptors):
        self.raw_type_descriptors = type_descriptors
        self.type_descriptor_map = {model.name: model for model in self.raw_type_descriptors}

    @property
    def type_descriptors(self):
        # filter out inline structs
        return [model for model in self.raw_type_descriptors if not hasattr(model, 'disposition') or 'inline' != model.disposition]

    def apply_attributes(self):
        """Sets properties from attributes within all structures."""
        for model in self._structs():
            for field in model.fields:
                if not hasattr(field, 'attributes') or not field.attributes:
                    continue

                for attribute in field.attributes:
                    if not hasattr(field.field_type, attribute.name):
                        raise AstException(f'field {field.name} ({field.field_type}) does not have property {attribute.name}')

                    setattr(field.field_type, attribute.name, attribute.value)

    def _structs(self):
        return [model for _, model in self.type_descriptor_map.items() if isinstance(model, Struct)]

    def expand_named_inlines(self):
        """Expands named inline fields within all structures."""

        for model in self._structs_with_named_inlines():
            original_fields = model.fields[:]
            model.fields = []

            for field in original_fields:
                if not self._is_named_inline(field):
                    model.fields.append(field)
                    continue

                if field.field_type not in self.type_descriptor_map:
                    raise AstException(f'struct {model.name} contains named inline of unknown type {field.field_type}')

                referenced_type_model = self.type_descriptor_map[field.field_type]
                model.fields.extend(referenced_type_model.apply_inline_template(field))

    def _structs_with_named_inlines(self):
        return [
            model for _, model in self.type_descriptor_map.items()
            if isinstance(model, Struct) and any(self._is_named_inline(field) for field in model.fields)
        ]

    @staticmethod
    def _is_named_inline(field):
        return hasattr(field, 'disposition') and 'inline' == field.disposition and hasattr(field, 'name')

    def expand_unnamed_inlines(self):
        """Expands unnamed inline fields within all structures."""

        for model in self._structs_with_unnamed_inlines():
            while self._has_unnamed_inline_field(model):
                original_fields = model.fields[:]
                model.fields = []

                for field in original_fields:
                    if not isinstance(field, StructInlinePlaceholder):
                        model.fields.append(field)
                        continue

                    if field.inlined_typename not in self.type_descriptor_map:
                        raise AstException(f'struct {model.name} contains unnamed inline of unknown type {field.inlined_typename}')

                    referenced_type_model = self.type_descriptor_map[field.inlined_typename]
                    if 'abstract' == referenced_type_model.disposition:
                        model.factory_type = referenced_type_model.name
                    elif referenced_type_model.factory_type:
                        model.factory_type = referenced_type_model.factory_type

                    model.fields.extend(referenced_type_model.fields)
                    if referenced_type_model.attributes:
                        if not model.attributes:
                            model.attributes = []

                        model.attributes.extend(referenced_type_model.attributes)

    def _structs_with_unnamed_inlines(self):
        return [
            model for _, model in self.type_descriptor_map.items()
            if self._has_unnamed_inline_field(model)
        ]

    @staticmethod
    def _has_unnamed_inline_field(model):
        return isinstance(model, Struct) and any(isinstance(field, StructInlinePlaceholder) for field in model.fields)
