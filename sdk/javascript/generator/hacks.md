# generator hacks

Hacks done so far:
 * filtering out `_reserved` fields
 * minor hack: when processing conditional fields, that depend on the field that is yet deserialized it is assumed, that those conditional fields are of equal size (only such case is in symbol's `NamespaceRegistrationTransactionBody`)
 * `embedded_` is skipped from transaction types, when generating `EmbeddedTransationFactory.create_by_name`

JS specific things:
 * to make code bit nicer, there's a minor hack around generating serialization for typed arrays (buffer name is passed down as argument to
   `.store()`)
