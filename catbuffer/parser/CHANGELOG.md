# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [3.3.0] - 07-Apr-2023

### Added
- support for generating both aligned and unaligned (de)serializers for structs (Javascript optimization)
- struct attribute: comparer
- field attribute: sizeref

### Changed
- extend alignment attribute to specify last element padding

## [3.2.0] - 18-Feb-2022

### Added
- add 'is_bitwise' enum attribute to indicate than an enumeration contains flags and should support bitwise operations
- generators module providing utility functions for generator authors
- struct attribute: is_aligned

### Changed
- added back one-pass code generation support and deprecated YAML generation
- rename 'implicit_size' attribute to 'is_size_implicit'
- added 'pad_last' and 'not pad_last' qualifiers to 'alignment' attribute

### Fixed
- serialization of signed integers (#7)

### Removed
- YAML generation

## [3.1.0] - 22-Dec-2021

### Added
- struct attribute: implicit_size
- struct field sizeof directive

### Changed
- struct discriminator attribute supports multiple values

## [3.0.2] - 17-Dec-2021

### Added
- struct attributes: {size, initializes, discriminator}
- struct field attributes: {is_byte_constrained, alignment, sort_key}
- struct "abstract" modifier
- AST object model
- CATS DSL documentation

### Changed
- rewrite entire parser using Lark grammar
- array(...) syntax
- deprecated YAML output format

## [2.0.2] - 15-Sep-2021

### Fixed
 - actually generate proper YAML files

### Removed
 - one pass code generation

## [2.0.1] - 06-Sep-2021

### Added
 - YAML export option
 - 'inline struct' directive to express macro-like expansion
 - conditional support for array types

### Changed
 - enforce strict pythonic naming
 - parser output to YAML instead of python objects

## [2.0.0] - 02-Sep-2021

### Added
 - first class support for reserved fields with 'make_reserved' keyword
 - allow enum value names to be used as constant values
 - add 'not' keyword and support for negative conditions
 - allow numeric conditionals as well as enum conditionals
 - add support for numeric arrays

### Changed
 - replace `const` keyword with `make_const` to preserve a unified left to right flow
 - rename keyword 'has' to 'in'
 - deprecate explicit use of 'byte' type

## [1.0.0] - 19-Apr-2021

### Added
 - initial code release

[3.3.0]: https://github.com/symbol/symbol/compare/catbuffer/parser/v3.2.0...catbuffer/parser/v3.3.0
[3.2.0]: https://github.com/symbol/symbol/compare/catbuffer/parser/v3.1.0...catbuffer/parser/v3.2.0
[3.1.0]: https://github.com/symbol/symbol/compare/catbuffer/parser/v3.0.2...catbuffer/parser/v3.1.0
[3.0.2]: https://github.com/symbol/symbol/compare/catbuffer/parser/v1.0.0...catbuffer/parser/v3.0.2
[1.0.0]: https://github.com/symbol/symbol/releases/tag/catbuffer/parser/v1.0.0
