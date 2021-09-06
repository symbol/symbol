# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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

[2.0.1]: https://github.com/symbol/catbuffer-parser/compare/v2.0.0...v2.0.1
[2.0.0]: https://github.com/symbol/catbuffer-parser/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/symbol/catbuffer-parser/releases/tag/v1.0.0
