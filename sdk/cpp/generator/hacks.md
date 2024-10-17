# generator hacks

Hacks done so far:
 * filtering out `_reserved` fields
 * minor hack: when processing conditional fields, that depend on the field that is yet deserialized it is assumed, that those conditional fields are of equal size (only such case is in symbol's `NamespaceRegistrationTransactionBody`)
 * `embedded_` is skipped from transaction types, when generating `EmbeddedTransationFactory.create_by_name`

Py specific things:
 * fields that use python keywords: 'size', 'type' and 'property' are altered to 'size_', 'type_', 'property_'
 * use 'self.value' as enum field when serializing (done inside EnumObject, probably should be elsewhere)
