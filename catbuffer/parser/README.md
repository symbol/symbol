# catbuffer

[![Build Status](https://api.travis-ci.com/symbol/catbuffer-parser.svg?branch=main)](https://travis-ci.com/symbol/catbuffer-parser)

The catbuffer library defines the protocol to serialize and deserialize Symbol entities. Code generators from the [catbuffer-generators](https://github.com/symbol/catbuffer-generators) project can then produce the leanest code necessary to serialize and deserialize those entities.

Using catbuffer-generators, developers can generate builder classes for a given set of programming languages. For example, the [Symbol SDKs](https://symbol.github.io/sdk) use the generated code to interact with the entities in binary form before announcing them to the network.

The [schemas](schemas) folder contains definitions for each entity's data structure. These definitions are always kept up to date and in sync with the [catapult server](https://github.com/symbol/catapult-server) code.

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

| Option               | Description                                                                                                                                                | Default       |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------- |
| -s, --schema TEXT    | Input CATS file                                                                                                                                            |               |
| -o, --output TEXT    | Output directory                                                                                                                                           | _generated    |
| -i, --include TEXT   | Schema root directory                                                                                                                                      | ./schemas     |
| -g, --generator TEXT | Generator to use to produce output files (see the [available generators](https://github.com/symbol/catbuffer-generators/blob/main/generators/All.py#L4)). |               |
| -c, --copyright TEXT | File containing copyright data to use with output files.                                                                                                   | ../HEADER.inc |

## Examples

In order to produce any output file, the [catbuffer-generators](https://github.com/symbol/catbuffer-generators) project is needed. Please see this project's usage examples.

However, ``catbuffer-parser`` can still be used on its own to parse input files and check their validity:

```bash
git clone --depth 1 --branch v2.0.0a https://github.com/symbol/catbuffer-schemas.git
python3 -m catparser --schema ../catbuffer-schemas/symbol/transfer/transfer.cats --include ../catbuffer-schemas/symbol
```
