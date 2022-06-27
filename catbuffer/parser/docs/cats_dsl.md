# CATS DSL

CATS are composed of three primary declarations: `using`, `enum`, `struct`.
Schemas are whitespace significant.

## import

CATS files can include other files by using the `import` statement.
For example, to import a file called "other.cats":
```cpp
import "other.cats"
```

All imported filenames are relative to the include path passed to the parser.
Using the CLI tool, this can be specified by the `--include` command line argument.

## using

Alias statements can be used to name a unique POD type.
CATS supports two types of built-ins:

- Integer types: unsigned ("uint") and signed ("int") for sizes { 1, 2, 4, 8 }.
For example, to define a `Height` type that is represented by a 8 byte unsigned integer:
```cpp
using Height = uint64
```

- Fixed buffer types: unsigned byte buffer of any length.
For example, to define a `PublicKey` type that is represented by a 32 byte unsigned buffer:
```cpp
using PublicKey = binary_fixed(32)
```

Importantly, each alias will be treated as a unique type and cannot be used interchangeably.
For example, `Weight` cannot be used where `Height` is expected and vice versa.
```cpp
using Height = uint64
using Weight = uint64
```

## enum

Enumeration statements can be used to define a set of possible values.
Each enumeration specifies an integer backing type.

For example, to define a `TransportMode` enumeration backed by a 2 byte unsigned integer:
```cpp
enum TransportMode : uint16
```

Values making up an enumeration follow the enumeration declaration on indented lines.
For example, to add three values to the `TransportMode` enumeration named `ROAD` (value 0x0001) and `SEA` (0x0002) and `SKY` (0x0004):
```cpp
enum TransportMode : uint32
	ROAD = 0x0001
	SEA = 0x0002
	SKY = 0x0004
```

### attributes

Hints can be attached to enumerations using attributes.

Enumerations support the following attributes:
1. `is_bitwise`: indicates that the enumeration represents flags and should support bitwise operations.

For example, to set the `is_bitwise` attribute on the `TransportMode` enumeration:
```cpp
@is_bitwise
enum TransportMode : uint32
	ROAD = 0x0001
	SEA = 0x0002
	SKY = 0x0004
```

## struct

Structure statements are used to define structured binary payloads.
Structure definition are comprehensive.
Unlike other formats, the CATS parser will never add extraneous data or padding.

Structures can have any of the following modifiers:
1. None: Generators are recommended to include the type in final output.
2. abstract: Generators are recommended to include the type in final output and produce corresponding factory.
3. inline: Generators are recommended to discard the structure from final output.

For example, to define a `Vehicle` struct with the `abstract` modifier:
```cpp
abstract struct Vehicle
```

Fields making up a structure follow the structure declaration on indented lines.
For example, to add an 8 byte unsigned  `weight` field of type to the `Vehicle` structure:
```cpp
abstract struct Vehicle
	weight = uint32
```

`make_const` can be used to define a const field, which does not appear in the struct layout.
For example, to define a 2 byte unsigned constant `TRANSPORT_MODE` with value `ROAD`:
```cpp
struct Car
	TRANSPORT_MODE = make_const(TransportMode, ROAD)
```

`make_reserved` can be used to define a reserved field, which does appear in the layout and specifies a default value.
For example, to define a 1 byte unsigned constant `wheel_count` with value 4:
```cpp
inline struct Car
	wheel_count = make_reserved(uint8, 4)
```

`sizeof` can be used to define a field that is filled with the size of another field.
For example, to define a 2 byte unsigned `car_size` that is filled with the size of the `car` field:
```cpp
inline struct SingleCarGarage
	car_size = sizeof(uint16, car)

	car = Car
```

### conditionals

Fields can be made conditional on the values of other fields.
The approximates the union concept present in some languages.
CATS supports the following operators:
1. `equals`: conditional field is included if reference field value matches condition value exactly
1. `not equals`: conditional field is included if reference field value does NOT match condition value
1. `has`: conditional field is included if reference field value has all of the condition flags set
1. `not has`: conditional field is included if reference field value does NOT have all of the condition flags set

For example, to indicate `buoyancy` is only present when `transport_mode` is equal to `SEA`:
```cpp
abstract struct Vehicle
	transport_mode = TransportMode

	buoyancy = uint32 if SEA equals transport_mode
```

### arrays

Dynamically sized arrays are supported.
Each array has an associated size that can be a constant, a property reference or a special `__FILL__` keyword.

For example, to define a `Garage` with a `vehicles` field that is composed of `vehicles_count` `Vehicle` structures:
```cpp
struct Garage
	vehicles_count = uint32

	vehicles = array(Vehicle, vehicles_count)
```

The special `__FILL__` keyword indicates that the array extends until the end of the structure.
In order for `__FILL__` to be used, the containing structure must contain a field containing its size in bytes, specified via the `@size` attribute.
For example, to indicate the `vehicles` array composed of `Vehicle` structures extends to the end of the `Garage` structure with byte size `garage_byte_size`:
```cpp
@size(garage_byte_size)
struct Garage
	garage_byte_size = uint32

	vehicles = array(Vehicle, __FILL__)
```

:warning: Array element types (`Vehicle` used in the examples) must either be fixed sized structures or variable sized structures with a `@size` attribute attached.

### inlines
A structure can be inlined within another using the `inline` keyword.
For example, to inline `Vehicle` at the start of a `Car` structure with two fields:
```cpp
struct Car
	inline Vehicle

	max_clearance = Height
	has_left_steering_wheel = uint8
```

Inlines are expanded where they appear, so the order of `Car` fields will be: {weight, max_clearance, has_left_steering_wheel}.
The expansion will be equivalent to:
```cpp
struct Car
	weight = uint32

	max_clearance = Height
	has_left_steering_wheel = uint8
```

In addition, a named inline will inline a referenced structure's fields with a prefix.
For example, in the following `SizePrefixedString` is inlined in `Vehicle` as `friendly_name`:
```cpp
inline struct SizePrefixedString
	size = uint32
	__value__ = array(int8, size)

abstract struct Vehicle
	weight = uint32

	friendly_name = inline SizePrefixedString

	year = uint16
```

The expansion will be equivalent to:
```cpp
abstract struct Vehicle
	weight = uint32

	friendly_name_size = uint32
	friendly_name = array(int8, friendly_name_size)

	year = uint16
```

Within the inlined structure, `__value__` is a special field name that will be replaced with the name (`friendly_name`) used in the containing structure (`Vehicle`).
All other fields in the inlined structure will have names prepended with the name (`friendly_name`) used in the containing structure (`Vehicle`) and an underscore.
So, `__value__` becomes `friendly_name` and `size` becomes  `friendly_name ` + `_` + `size` or `friendly_name_size`.

### attributes

Hints can be attached to structures using attributes.

Structures support the following attributes:
1. `is_aligned`: indicates that all structure fields are positioned on aligned boundaries.
1. `is_size_implicit`: indicates that the structure could be referenced in a `sizeof(x)` statement and must support a size calculation.
1. `size(x)`: indicates that the `x` field contains the full size of the (variable sized) structure.
1. `initializes(x, Y)`: indicates that the `x` field should be initialized with the `Y` constant.
1. `discriminator(x [, y]+)`: indicates that the (`x`, ...`y`) properties should be used as the discriminator when generating a factory (only has meaning for abstract structures).
1. `comparer(x [!a] [, y [!b]])`: indicates that the (`x`, ...`y`) properties should be used for custom sorting. optional (`a`, ...` b`) transforms can be specified and applied prior to property comparison. currently, the only transform supported is `ripemd_keccak_256` for backwards compatibility with NEM.

For example, to link the `transport_mode` field with the `TRANSPORT_MODE` constant:
```cpp
@initializes(transport_mode, TRANSPORT_MODE)
abstract struct Vehicle
	transport_mode = TransportMode

struct Car
	TRANSPORT_MODE = make_const(TransportMode, ROAD)

	inline Vehicle
```

Notice that `TRANSPORT_MODE` can be defined in any derived structure.

Array fields support the following attributes:
1. `is_byte_constrained`: indicates the size value should be interpreted as a byte value instead of an element count.
1. `alignment(x [, [not] pad_last])`: indicates that elements should be padded so that they start on `x`-aligned boundaries.
1. `sort_key(x)`: indicates that elements within the array should be sorted by the `x` property.

When alignment is specified, by default, the final element is padded to end on an `x`-aligned boundary.
This can be made explicit by including the `pad_last` qualifier.
This can be disabled by including the `not pad_last` qualifier, which will not pad the last element to an `x`-aligned boundary.

For example, to sort vehicles by `weight`:
```cpp
struct Garage
	@sort_key(weight)
	vehicles = array(Vehicle, __FILL__)
```

Integer fields support the following attribute:
1. `sizeref(x [, y])`: indicates the field should be initialized with the size of the `x` property adjusted by `y`.

For example, to autopopulate `vehicle_size` with the size of itself and the vehicle field:
```cpp
struct Garage
	@sizeref(vehicle, 2)
	vehicle_size = uint16

	vehicle = Vehicle
```

## comments

Any line starting with a `#` is treated as a comment.
If a comment line is directly above a declaration or sub-declaration it is treated as documentation and preserved by the parser.
Otherwise, it is discarded.

For example, in the following "comment 1" is discarded while "comment 2 comment 3" is extracted as the documentation for Height.
```python
# comment 1

# comment 2
# comment 3
using Height = uint64
```
