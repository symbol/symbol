# catbuffer parser

[![Build Status](https://api.travis-ci.com/symbol/catbuffer-parser.svg?branch=main)](https://travis-ci.com/symbol/catbuffer-parser)

The catbuffer library defines the protocol to serialize and deserialize Symbol entities. Code generators from the [catbuffer-generators](https://github.com/symbol/catbuffer-generators) project can then produce the leanest code necessary to serialize and deserialize those entities.

Using catbuffer-generators, developers can generate builder classes for a given set of programming languages. For example, the [Symbol SDKs](https://symbol.github.io/sdk) use the generated code to interact with the entities in binary form before announcing them to the network.

The [schemas](https://github.com/symbol/catbuffer-schemas) folder contains definitions for each entity's data structure. These definitions are always kept up to date and in sync with the [catapult server](https://github.com/symbol/catapult-server) code.

## Requirements

* Python >= 3.6

## Installation

1. Clone the ``catbuffer-parser`` repository:

```bash
git clone https://github.com/symbol/catbuffer-parser
```

2. (optional) Install lint requirements:

```bash
pip3 install -r lint_requirements.txt
```

## Usage

```bash
python3 -m catparser [OPTIONS]
```

| Option               | Description                                                                                                                                                |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -s, --schema TEXT    | Input CATS file                                                                                                                                            |
| -i, --include TEXT   | Schema root directory                                                                                                                                      |
| -o, --output TEXT    | YAML output file (optional)                                                                                                                                |

## Examples

``catparser`` can be used on its own to parse input files, check their validity and optionally output a YAML file containing the parsed type descriptors:

```bash
git clone --depth 1 --branch v2.0.0a https://github.com/symbol/catbuffer-schemas.git

# parse but don't output anything
python3 -m catparser --schema ../catbuffer-schemas/symbol/transfer/transfer.cats --include ../catbuffer-schemas/symbol

# parse and output a YAML file
python3 -m catparser --schema ../catbuffer-schemas/symbol/transfer/transfer.cats --include ../catbuffer-schemas/symbol --output ../catbuffer-schemas/all.yaml
```
