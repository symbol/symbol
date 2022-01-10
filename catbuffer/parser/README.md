# catbuffer-parser

[![Build Status](https://api.travis-ci.com/symbol/catbuffer-parser.svg?branch=main)](https://travis-ci.com/symbol/catbuffer-parser)

This project is the reference parser implementation for the CATS (Concise Affinitized Transfer Schema) DSL that is used by the Symbol blockchain. The parser converts a CATS file or files into an AST that can be used by a generator to produce serialization and deserialzation code for defined objects.

- A reference to the DSL format can be found [here](docs/cats_dsl.md).
- For a sampling of real world schemas, those used by the Symbol blockchain can be found [here](https://github.com/symbol/catbuffer-schemas).

## Requirements

* Python >= 3.6

## Installation

```bash
git clone https://github.com/symbol/catbuffer-parser
pip3 install -r requirements.txt
pip3 install -r lint_requirements.txt # optional
```

## Usage

```
usage: python -m catparser [-h] -s SCHEMA -i INCLUDE [-o OUTPUT]

CATS code generator

optional arguments:
  -h, --help            show this help message and exit
  -s SCHEMA, --schema SCHEMA
                        input CATS file
  -i INCLUDE, --include INCLUDE
                        schema root directory
  -o OUTPUT, --output OUTPUT
                        yaml output file
```

## Examples

``catparser`` can be used on its own to parse input files, check their validity and optionally output a YAML file containing the parsed type descriptors:

```bash
git clone --depth 1 --branch v3.0.0 https://github.com/symbol/catbuffer-schemas.git

# parse but don't output anything
python3 -m catparser --schema ../catbuffer-schemas/symbol/transfer/transfer.cats --include ../catbuffer-schemas/symbol

# parse and output a YAML file (deprecated)
python3 -m catparser --schema ../catbuffer-schemas/symbol/transfer/transfer.cats --include ../catbuffer-schemas/symbol --output ../catbuffer-schemas/all.yaml
```
