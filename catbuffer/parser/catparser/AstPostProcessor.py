from .ast import AstException, Struct


class AstPostProcessor:
    """Post processes AST type descriptors."""

    def __init__(self, type_descriptors):
        self.raw_type_descriptors = type_descriptors
        self.type_descriptor_map = {model.name: model for model in self.raw_type_descriptors}

    @property
    def type_descriptors(self):
        # filter out inline structs
        return [model for model in self.raw_type_descriptors if not hasattr(model, 'disposition') or 'inline' != model.disposition]

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
