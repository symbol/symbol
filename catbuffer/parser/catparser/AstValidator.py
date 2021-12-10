from .ast import Array, Conditional, Enum, Struct, StructInlinePlaceholder


class ErrorDescriptor:
    """Describes an AST validation error."""

    def __init__(self, message, typename, field_names=None):
        self.message = message
        self.typename = typename
        self.field_names = [field_names] if isinstance(field_names, str) else field_names

    def __eq__(self, rhs):
        return isinstance(rhs, ErrorDescriptor) and str(self) == str(rhs)

    def __repr__(self):
        if self.field_names:
            joined_field_names = ', '.join(self.field_names)
            return f'[{self.typename}::{{ {joined_field_names} }}] {self.message}'

        return f'[{self.typename}] {self.message}'


class AstValidator:
    """Validates AST type descriptors."""

    def __init__(self, type_descriptors):
        self.raw_type_descriptors = type_descriptors
        self.type_descriptor_map = {model.name: model for model in self.raw_type_descriptors}

        self.errors = []

    def validate(self):
        """Validates all types for correctness."""

        for _, model in self.type_descriptor_map.items():
            if isinstance(model, Enum):
                self._validate_enum(model)

            if isinstance(model, Struct):
                self._validate_struct(model)

    def _validate_enum(self, model):
        duplicate_names = self._find_duplicate_names(model.values)
        if duplicate_names:
            self.errors.append(ErrorDescriptor('duplicate enum values', model.name, duplicate_names))

    def _validate_struct(self, model):
        duplicate_names = self._find_duplicate_names(model.fields)
        if duplicate_names:
            self.errors.append(ErrorDescriptor('duplicate struct fields', model.name, duplicate_names))

        field_map = {field.name: field for field in model.fields if hasattr(field, 'name')}
        for field in model.fields:
            def create_error_descriptor(message, error_field=field):
                return ErrorDescriptor(message, model.name, error_field.name)

            if isinstance(field, StructInlinePlaceholder):
                self._validate_unnamed_inline(field, lambda message: ErrorDescriptor(message, model.name))
                continue

            if not self._is_known_type(field.field_type):
                self.errors.append(create_error_descriptor(f'reference to unknown type {field.field_type}'))
            else:
                if 'inline' == field.disposition and 'inline' != self.type_descriptor_map[field.field_type].disposition:
                    self.errors.append(create_error_descriptor(f'named inline field referencing non inline struct {field.field_type}'))

            if isinstance(field.field_type, Array):
                self._validate_array(field.field_type, field_map, create_error_descriptor)

            if field.value is not None:
                if isinstance(field.value, Conditional):
                    self._validate_conditional(field, field_map, create_error_descriptor)
                else:
                    self._validate_in_range(field.field_type, field.value, create_error_descriptor)

    def _validate_unnamed_inline(self, field, create_error_descriptor):
        if not self._is_known_type(field.inlined_typename):
            self.errors.append(create_error_descriptor(f'reference to unknown inlined type {field.inlined_typename}'))
        else:
            if 'inline' == self.type_descriptor_map[field.inlined_typename].disposition:
                self.errors.append(create_error_descriptor(f'unnamed inline referencing inline struct {field.inlined_typename}'))

    def _validate_array(self, field_type, field_map, create_error_descriptor):
        element_type = field_type.element_type
        is_sort_key_valid = True
        sort_key = field_type.sort_key
        if not self._is_known_type(element_type):
            self.errors.append(create_error_descriptor(f'reference to unknown element type {element_type}'))
            is_sort_key_valid = not sort_key
        else:
            is_sort_key_valid = not sort_key or any(
                sort_key == element_field.name for element_field in self.type_descriptor_map[element_type].fields
            )

        if not is_sort_key_valid:
            self.errors.append(create_error_descriptor(f'reference to unknown sort_key property {sort_key}'))

        size = field_type.size
        if isinstance(size, str) and size not in field_map:
            self.errors.append(create_error_descriptor(f'reference to unknown size property {size}'))

    def _validate_conditional(self, field, field_map, create_error_descriptor):
        linked_field_name = field.value.linked_field_name

        if linked_field_name not in field_map:
            self.errors.append(create_error_descriptor(f'reference to unknown condition field {linked_field_name}'))
        else:
            self._validate_in_range(field_map[linked_field_name].field_type, field.value.value, create_error_descriptor)

    def _validate_in_range(self, value_type, value, create_error_descriptor):
        if isinstance(value_type, str):
            if self._is_known_type(value_type):
                value_type = self.type_descriptor_map[value_type]

                if isinstance(value_type, Enum):
                    if not any(value == enum_value.name for enum_value in value_type.values):
                        self.errors.append(create_error_descriptor(f'field value "{value}" is not a valid enum value'))

                    return

        if not isinstance(value, int):
            self.errors.append(create_error_descriptor(f'field value "{value}" is not a valid numeric value'))

    def _is_known_type(self, typename):
        return not isinstance(typename, str) or typename in self.type_descriptor_map

    @staticmethod
    def _find_duplicate_names(items):
        unique_names = set()
        duplicate_names = set()
        for item in filter(lambda item: hasattr(item, 'name'), items):
            (unique_names if item.name not in unique_names else duplicate_names).add(item.name)

        return duplicate_names
