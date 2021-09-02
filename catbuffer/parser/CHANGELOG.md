# Changelog
All notable changes to this project will be documented in this file.

The changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.0] - 19-Apr-2021

### Added
 - initial code release

## [2.0.0] - 30-Aug-2021

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

[2.0.0]: https://github.com/symbol/catbuffer-parser/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/symbol/catbuffer-parser/releases/tag/v1.0.0
