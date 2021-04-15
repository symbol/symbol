import unittest

from catbuffer_parser.CatsParser import CatsParseException, CatsParser


def parse_all(lines, imports=None):
    # Act:
    if imports is None:
        imports = []

    parser = CatsParser(imports.append)
    for line in lines:
        parser.process_line(line)

    return parser.type_descriptors()


def uint_descriptor(size, explicit_type=True):
    descriptor = {'signedness': 'unsigned', 'size': size}
    if explicit_type:
        descriptor['type'] = 'byte'

    return descriptor


def int_descriptor(size, explicit_type=True):
    descriptor = uint_descriptor(size, explicit_type)
    descriptor['signedness'] = 'signed'
    return descriptor


class CatsParserTests(unittest.TestCase):
    # pylint: disable=too-many-public-methods

    # region utils

    def _assert_parse_delayed_exception(self, lines):
        # Arrange:
        imports = []
        parser = CatsParser(imports.append)
        for line in lines[:-1]:
            parser.process_line(line)

        # Act + Assert:
        with self.assertRaises(CatsParseException):
            parser.process_line(lines[-1])

    def _assert_parse_commit_exception(self, lines):
        # Arrange:
        imports = []
        parser = CatsParser(imports.append)
        for line in lines:
            parser.process_line(line)

        # Act + Assert:
        with self.assertRaises(CatsParseException):
            parser.commit()

    # endregion

    # region empty + basic

    def test_no_types_are_exposed_initially(self):
        # Act:
        parser = CatsParser(None)

        # Assert:
        self.assertEqual(0, len(parser.type_descriptors()))

    def test_no_types_are_extracted_from_blank_lines(self):
        # Act:
        type_descriptors = parse_all([
            '',
            '',
            ''
        ])

        # Assert:
        self.assertEqual(0, len(type_descriptors))

    def test_parse_is_aborted_on_unknown_line(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'using Foo = uint8',
            'alias Bar = uint8'
        ])

    # endregion

    # region comments

    def test_unattached_comments_are_ignored(self):
        # Act:
        type_descriptors = parse_all([
            '# comment alias',
            '# another comment',
            '',  # should clear previous comments
            'using MosaicId = uint64'
        ])

        # Assert:
        self.assertEqual(1, len(type_descriptors))
        self.assertEqual(type_descriptors['MosaicId'], {**uint_descriptor(8), 'comments': ''})

    def test_previously_attached_comments_are_ignored(self):
        # Act:
        type_descriptors = parse_all([
            '# comment one',
            'using Age = uint64',
            '# comment two',
            'using Year = uint16'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Age'], {**uint_descriptor(8), 'comments': 'comment one'})
        self.assertEqual(type_descriptors['Year'], {**uint_descriptor(2), 'comments': 'comment two'})

    # endregion

    # region imports

    def test_can_parse_valid_import(self):
        # Act:
        imports = []
        type_descriptors = parse_all(['import "foo.cats"'], imports)

        # Assert:
        self.assertEqual(0, len(type_descriptors))
        self.assertEqual(['foo.cats'], imports)

    # endregion

    # region alias

    def test_can_parse_alias(self):
        # Act:
        type_descriptors = parse_all([
            'using MosaicId = uint64',
            '# unique account identifier',
            'using Address = binary_fixed(25)'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['MosaicId'], {**uint_descriptor(8), 'comments': ''})
        self.assertEqual(type_descriptors['Address'], {**uint_descriptor(25), 'comments': 'unique account identifier'})

    # endregion

    # region struct

    def test_can_parse_struct_builtin_types(self):
        # Act:
        type_descriptors = parse_all([
            '# binary layout for a pair',
            'struct Pair',
            '\t# some field comment',
            '\tfooBar = uint64',
            '\tbaz = binary_fixed(25)'
        ])

        # Assert:
        self.assertEqual(1, len(type_descriptors))
        self.assertEqual(type_descriptors['Pair'], {'type': 'struct', 'comments': 'binary layout for a pair', 'layout': [
            {'name': 'fooBar', **uint_descriptor(8), 'comments': 'some field comment'},
            {'name': 'baz', **uint_descriptor(25), 'comments': ''}
        ]})

    def test_can_parse_struct_custom_types(self):
        # Act:
        type_descriptors = parse_all([
            'using MosaicId = uint16',
            'using Amount = uint16',
            '# binary layout for a mosaic',
            'struct Mosaic',
            '\t# mosaic identifier',
            '\tmosaicId = MosaicId',
            '\tamount = Amount'
        ])

        # Assert:
        self.assertEqual(3, len(type_descriptors))
        self.assertEqual(type_descriptors['Mosaic'], {'type': 'struct', 'comments': 'binary layout for a mosaic', 'layout': [
            {'name': 'mosaicId', 'type': 'MosaicId', 'comments': 'mosaic identifier'},
            {'name': 'amount', 'type': 'Amount', 'comments': ''}
        ]})

    def test_can_parse_struct_conditional_types(self):
        # Act:
        type_descriptors = parse_all([
            'enum Shape : uint8',
            '\tcircle = 4',
            '\trectangle = 9',
            'using Circ = uint16',
            'using Perm = uint16',
            'struct Enclosing',
            '\tdiscriminator = Shape',
            '\t# u part 1',
            '\tcircumference = Circ if discriminator has circle',
            '\t# union pt 2',
            '\tperimiter = Perm if discriminator equals rectangle'
        ])

        # Assert:
        self.assertEqual(4, len(type_descriptors))
        self.assertEqual(type_descriptors['Enclosing'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'discriminator', 'type': 'Shape', 'comments': ''},
            {
                'name': 'circumference', 'type': 'Circ',
                'condition': 'discriminator', 'condition_value': 'circle', 'condition_operation': 'has',
                'comments': 'u part 1'
            },
            {
                'name': 'perimiter', 'type': 'Perm',
                'condition': 'discriminator', 'condition_value': 'rectangle', 'condition_operation': 'equals',
                'comments': 'union pt 2'
            }
        ]})

    def test_can_parse_struct_conditional_types_with_inline_member(self):
        # Act:
        type_descriptors = parse_all([
            'enum Shape : uint8',
            '\tcircle = 4',
            '\trectangle = 9',
            'using Circ = uint16',
            'using Perm = uint16',
            'struct Version',
            '\tversion = uint16',
            'struct Enclosing',
            '\tinline Version',
            '\tdiscriminator = Shape',
            '\t# u part 1',
            '\tcircumference = Circ if discriminator has circle',
            '\t# union pt 2',
            '\tperimiter = Perm if discriminator equals rectangle'
        ])

        # Assert:
        self.assertEqual(5, len(type_descriptors))
        self.assertEqual(type_descriptors['Enclosing'], {'type': 'struct', 'comments': '', 'layout': [
            {'type': 'Version', 'disposition': 'inline', 'comments': ''},
            {'name': 'discriminator', 'type': 'Shape', 'comments': ''},
            {
                'name': 'circumference', 'type': 'Circ',
                'condition': 'discriminator', 'condition_value': 'circle', 'condition_operation': 'has',
                'comments': 'u part 1'
            },
            {
                'name': 'perimiter', 'type': 'Perm',
                'condition': 'discriminator', 'condition_value': 'rectangle', 'condition_operation': 'equals',
                'comments': 'union pt 2'
            }
        ]})

    def test_can_parse_struct_conditional_types_trailing_discriminator(self):
        # Act:
        type_descriptors = parse_all([
            'enum Shape : uint8',
            '\tcircle = 4',
            '\trectangle = 9',
            'using Circ = uint16',
            'using Perm = uint16',
            'struct Enclosing',
            '\t# u part 1',
            '\tcircumference = Circ if discriminator has circle',
            '\t# union pt 2',
            '\tperimiter = Perm if discriminator equals rectangle',
            '\tdiscriminator = Shape'
        ])

        # Assert:
        self.assertEqual(4, len(type_descriptors))
        self.assertEqual(type_descriptors['Enclosing'], {'type': 'struct', 'comments': '', 'layout': [
            {
                'name': 'circumference', 'type': 'Circ',
                'condition': 'discriminator', 'condition_value': 'circle', 'condition_operation': 'has',
                'comments': 'u part 1'
            },
            {
                'name': 'perimiter', 'type': 'Perm',
                'condition': 'discriminator', 'condition_value': 'rectangle', 'condition_operation': 'equals',
                'comments': 'union pt 2'
            },
            {'name': 'discriminator', 'type': 'Shape', 'comments': ''}
        ]})

    def test_can_parse_struct_array_types(self):
        # Act:
        type_descriptors = parse_all([
            'using Truck = uint16',
            'using Car = uint16',
            'struct Fleet',
            '\tcarCount = uint8',
            '# all trucks in the fleet',
            '\ttrucks = array(Truck, 10)',
            '\tcars = array(Car, carCount)'
        ])

        # Assert:
        self.assertEqual(3, len(type_descriptors))
        self.assertEqual(type_descriptors['Fleet'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'carCount', **uint_descriptor(1), 'comments': ''},
            {'name': 'trucks', 'type': 'Truck', 'size': 10, 'comments': 'all trucks in the fleet'},
            {'name': 'cars', 'type': 'Car', 'size': 'carCount', 'comments': ''}
        ]})

    def test_can_parse_struct_sorted_array_types(self):
        # Act:
        type_descriptors = parse_all([
            'struct Face',
            '\teyeColor = uint8',
            'struct Tracking',
            '\tfaces = array(Face, 10, sort_key=eyeColor)'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Tracking'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'faces', 'type': 'Face', 'size': 10, 'sort_key': 'eyeColor', 'comments': ''}
        ]})

    def test_can_parse_struct_vararray_types(self):
        # Act:
        type_descriptors = parse_all([
            'using Car = uint16',
            'struct Fleet',
            '\tcarsSize = uint8',
            '# all cars in the fleet',
            '\tcars = array(Car, size=carsSize)'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Fleet'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'carsSize', **uint_descriptor(1), 'comments': ''},
            {'name': 'cars', 'type': 'Car', 'size': 'carsSize', 'disposition': 'var', 'comments': 'all cars in the fleet'}
        ]})

    def test_can_parse_struct_array_fill_types(self):
        # Act:
        type_descriptors = parse_all([
            'using Car = uint16',
            'struct Fleet',
            '# all cars in the fleet',
            '\tcars = array(Car, __FILL__)'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Fleet'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'cars', 'type': 'Car', 'size': 0, 'disposition': 'fill', 'comments': 'all cars in the fleet'}
        ]})

    def test_cannot_parse_struct_vararray_numeric_size_types(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'using Car = uint16',
            'struct Fleet',
            '# all cars in the fleet',
            '\tcars = array(Car, size=123)'
        ])

    def test_cannot_parse_struct_vararray_fill_types(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'using Car = uint16',
            'struct Fleet',
            '# all cars in the fleet',
            '\tcars = array(Car, size=__FILL__)'
        ])

    def test_can_parse_struct_closed_by_other_type(self):
        # Act:
        type_descriptors = parse_all([
            'struct Mosaic',
            '\tmosaicId = uint64',
            '',
            '\tamount = uint32',
            'using Address = binary_fixed(25)'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Mosaic'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'mosaicId', **uint_descriptor(8), 'comments': ''},
            {'name': 'amount', **uint_descriptor(4), 'comments': ''}
        ]})

    def test_can_parse_struct_with_inline_member(self):
        # Act:
        type_descriptors = parse_all([
            'struct Placeholder',
            'struct Pair',
            '\tfooBar = uint64',
            '# some placeholder comment',
            '\tinline Placeholder',
            '\tbaz = uint32'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Pair'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'fooBar', **uint_descriptor(8), 'comments': ''},
            {'type': 'Placeholder', 'disposition': 'inline', 'comments': 'some placeholder comment'},
            {'name': 'baz', **uint_descriptor(4), 'comments': ''}
        ]})

    def test_can_parse_struct_with_const_member(self):
        # Act:
        type_descriptors = parse_all([
            'struct Pair',
            '\tfooBar = uint64',
            '# some const comment',
            '\tconst int8 tupleSize = 2',
            '\tbaz = uint32'
        ])

        # Assert:
        self.assertEqual(1, len(type_descriptors))
        self.assertEqual(type_descriptors['Pair'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'fooBar', **uint_descriptor(8), 'comments': ''},
            {'name': 'tupleSize', **int_descriptor(1), 'disposition': 'const', 'value': 2, 'comments': 'some const comment'},
            {'name': 'baz', **uint_descriptor(4), 'comments': ''}
        ]})

    def test_cannot_parse_struct_with_unknown_member_type(self):
        # Act + Assert:
        for type_name in ['MosaicId', 'array(MosaicId, 10)']:
            self._assert_parse_delayed_exception([
                'struct Foo',
                '\tid = {0}'.format(type_name)
            ])

    def test_cannot_parse_struct_with_non_enum_condition_link(self):
        # Act + Assert:
        self._assert_parse_commit_exception([
            'using Shape = uint8',
            'using Circ = uint16',
            'struct Enclosing',
            '\tdiscriminator = Shape',
            '\tcircumference = Circ if discriminator equals 123'
        ])

    def test_cannot_parse_struct_with_unknown_condition_value(self):
        # Act + Assert:
        self._assert_parse_commit_exception([
            'enum Shape : uint8',
            '\tcircle = 1',
            'using Circ = uint16',
            'struct Enclosing',
            '\tdiscriminator = Shape',
            '\tcircumference = Circ if discriminator equals hexagon'
        ])

    def test_cannot_parse_struct_with_unknown_array_size(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'using MosaicId = uint16',
            'struct Foo',
            '\tids = array(MosaicId, numMosaics)'
        ])

    def test_cannot_parse_struct_with_unknown_sort_key(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'using Face = uint16',
            'struct Tracking',
            '\tfaces = array(Face, 10, sort_key=eyeColor)'
        ])

    def test_cannot_parse_struct_with_unknown_inline_type(self):
        # Act + Assert:
        for type_name in ['MosaicId', 'array(MosaicId, 10)', 'uint8', 'binary_fixed(25)']:
            self._assert_parse_delayed_exception([
                'struct Foo',
                '\tinline {0}'.format(type_name)
            ])

    def test_cannot_parse_struct_with_unknown_const_type(self):
        # Act + Assert:
        for type_name in ['uint7', 'binary_fixed(25)', 'Car']:
            self._assert_parse_delayed_exception([
                'struct Foo',
                '\tconst {0} bar = 123'.format(type_name)
            ])

    # endregion

    # region enum

    def test_can_parse_enum_values(self):
        # Act:
        type_descriptors = parse_all([
            '# enumeration of entity types',
            'enum EntityType : uint16',
            '\t# transfer transaction type',
            '\ttransfer = 7',
            '\thashLock = 0x0C'
        ])

        # Assert:
        self.assertEqual(1, len(type_descriptors))
        self.assertEqual(type_descriptors['EntityType'], {
            'type': 'enum', **uint_descriptor(2, False), 'comments': 'enumeration of entity types', 'values': [
                {'name': 'transfer', 'value': 7, 'comments': 'transfer transaction type'},
                {'name': 'hashLock', 'value': 12, 'comments': ''}
            ]
        })

    def test_can_parse_enum_closed_by_other_type(self):
        # Act:
        type_descriptors = parse_all([
            'enum EntityType : uint16',
            '\ttransfer = 7',
            '',
            '\thashLock = 0x0C',
            'using Address = binary_fixed(25)'
        ])

        # Assert:
        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['EntityType'], {'type': 'enum', **uint_descriptor(2, False), 'comments': '', 'values': [
            {'name': 'transfer', 'value': 7, 'comments': ''},
            {'name': 'hashLock', 'value': 12, 'comments': ''}
        ]})

    def test_cannot_parse_enum_with_non_numeric_value(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'enum EntityType : uint16',
            '\ttransfer = QZ'
        ])

    # endregion

    # region scoping

    def test_cannot_parse_as_outer_scope_from_inner_scope(self):
        # Arrange:
        valid_headers = [
            ['struct Mosaic', '\tmosaicId = uint64'],
            ['enum EntityType : uint16', '\ttransfer = 7']
        ]

        # Act + Assert:
        # - using is not valid in composite scope
        for header in valid_headers:
            self._assert_parse_delayed_exception(header + ['\tusing Address = binary_fixed(25)'])

    def test_cannot_parse_schema_with_duplicate_type_names(self):
        # Arrange:
        valid_declarations = [
            ['struct Bar', '\tfoo = uint64'],
            ['enum Bar : uint16', '\tbaz = 7'],
            ['using Bar = uint16']
        ]

        # Act + Assert:
        counter = 0
        for declaration1 in valid_declarations:
            for declaration2 in valid_declarations:
                lines = declaration1 + declaration2
                if not declaration2[0].startswith('using'):
                    lines += ['using Baz = uint32']  # make sure all composites are closed

                self._assert_parse_delayed_exception(lines)
                counter += 1

        # Sanity:
        self.assertEqual(9, counter)

    def test_cannot_parse_schema_with_duplicate_struct_property_names_in_same_scope(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'struct Bar',
            '\tfoo = uint8',
            '\tfoo = uint16'
        ])

    def test_cannot_parse_schema_with_duplicate_enum_property_names_in_same_scope(self):
        # Act + Assert:
        self._assert_parse_delayed_exception([
            'enum Bar : uint16',
            '\tfoo = 4',
            '\tfoo = 9'
        ])

    def test_can_parse_schema_with_duplicate_struct_property_names_in_different_scopes(self):
        # Act:
        type_descriptors = parse_all([
            'struct Bar',
            '\tfoo = uint8',
            '',
            'struct Baz',
            '\tfoo = uint16'
        ])

        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Bar'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'foo', **uint_descriptor(1), 'comments': ''}
        ]})
        self.assertEqual(type_descriptors['Baz'], {'type': 'struct', 'comments': '', 'layout': [
            {'name': 'foo', **uint_descriptor(2), 'comments': ''}
        ]})

    def test_can_parse_schema_with_duplicate_enum_property_names_in_different_scopes(self):
        # Act:
        type_descriptors = parse_all([
            'enum Bar : uint16',
            '\tfoo = 4',
            '',
            'enum Baz : int32',
            '\tfoo = 9'
        ])

        self.assertEqual(2, len(type_descriptors))
        self.assertEqual(type_descriptors['Bar'], {'type': 'enum', **uint_descriptor(2, False), 'comments': '', 'values': [
            {'name': 'foo', 'value': 4, 'comments': ''}
        ]})
        self.assertEqual(type_descriptors['Baz'], {'type': 'enum', **int_descriptor(4, False), 'comments': '', 'values': [
            {'name': 'foo', 'value': 9, 'comments': ''}
        ]})

    # endregion

    # region ordering

    def test_type_definition_order_is_preserved(self):
        # Act:
        type_descriptors = parse_all([
            'using Truck = uint16',
            'struct Fleet',
            '\tcarCount = uint8',
            'enum Bar : uint16',
            '\tfoo = 4',
            'using Car = uint16'
        ])

        # Assert:
        self.assertEqual(4, len(type_descriptors))
        self.assertEqual(list(type_descriptors.keys()), ['Truck', 'Fleet', 'Bar', 'Car'])

    # endregion
