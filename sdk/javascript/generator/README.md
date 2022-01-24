# catbuffer generator

## Generate catbuffer

```sh
PYTHONPATH=../../catbuffer/parser python -m catparser \
   --schema ../../catbuffer/schemas/nem/all.cats  \
   --include ../../catbuffer/schemas/nem \
   --output ../../sdk/python/symbolchain/nc \
   --quiet \
   --generator generator.Generator

PYTHONPATH=../../catbuffer/parser python -m catparser \
   --schema ../../catbuffer/schemas/symbol/all.cats  \
   --include ../../catbuffer/schemas/symbol \
   --output ../../sdk/python/symbolchain/sc \
   --quiet \
   --generator generator.Generator
```

Run test vectors (assuming 'vectors' dir in currents directory):

```bash
PYTHONPATH=.. pytest -v
```

## Generator overview

1. Every type within the YAML file gets assigned one of the type objects: `EnumObject`, `ArrayObject`, `IntObject` or `StructObject`.
2. Of those types, only `StructObject` is more complicated and requires some further processing, described below.
3. Every type object has a corresponding type formatter, respectively:

   * `EnumObject` - `EnumTypeFormatter`.
   * `ArrayObject` and `IntObject` - `PodTypeFormatter`.
   * `StructObject` - `StructFormatter`.

Actual generation happens through the generic `TypeFormatter`, that uses the specific type formatters to generate output.

## Struct processing

1. Fields within structs get assigned one of the type objects depending on the field's `disposition` and `type`.
2. Fields get assigned proper printers, that handle how fields are handled when generating struct output:

   * `IntPrinter` - for simple types.
   * `ArrayPrinter` and `TypedArrayPrinter` for byte arrays and typed arrays respectively.
   * `BuiltinPrinter` - for builtin (already processed) types.

Every printer gets some "fixed" field names.
In case of Python there are two names that get fixed:

* `size` - Handled differently already.
* `type` - Not to clash with the `type()` builtin.

At the end of struct processing, 'size' fields are bound to variable-length fields.
For example, `TransferTransaction.message_size` is bound to `TransferTransaction.message`. This allows generating the following serialization for the `message_size` field:

```py
bytes_ += len(self._message).to_bytes(2, byteorder='little', signed=False)
```
