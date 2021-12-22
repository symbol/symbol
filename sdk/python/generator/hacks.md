# generator hacks

Hacks done so far:
 * filtering out `_reserved` fields
 * ~~hard-coded alignment value in ArrayHelpers.read_variable_size_elements~~
 * ~~there's alignment added when generating size() method, inside printers.TypedArrayPrinter.get_size~~
 * ~~discriminator name and discriminator value are hard-coded in AbstractImplMap~~
 * minor hack: when processing conditional fields, that depend on the field that is yet deserialized it is assumed, that those conditional fields are of equal size (only such case is in symbol's `NamespaceRegistrationTransactionBody`)
 * `embedded_` is skipped from transaction types, when generating `EmbeddedTransationFactory.create_by_name`

Py specific things:
 * fields like 'size' and 'type' are altered to 'size_' and 'type_'
 * use 'self.value' as enum field when serializing (done inside EnumObject, probably should be elsewhere)

~~TODO:~~
 * ~~yml is missing information how to assign types to resolution statements~~ - done by @Jaguar0625 via catparser-lark changes, along with Receipt changes
 * ~~remove transaction 'Body'-ies  (they are already inlined within proper txes)~~ - done by @Jaguar0625 via catparser changes
 * ~~version handling inside FactoryFormatter~~
