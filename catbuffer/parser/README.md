# catbuffer

[![Build Status](https://api.travis-ci.com/nemtech/catbuffer.svg?branch=main)](https://travis-ci.com/nemtech/catbuffer)

The catbuffer library defines the protocol to serialize and deserialize Symbol entities. Code generators from the [catbuffer-generators](https://github.com/nemtech/catbuffer-generators) project can then produce the leanest code necessary to serialize and deserialize those entities.

Using catbuffer-generators, developers can generate builder classes for a given set of programming languages. For example, the [Symbol SDKs](https://nemtech.github.io/sdk) use the generated code to interact with the entities in binary form before announcing them to the network.

The [schemas](schemas) folder contains definitions for each entity's data structure. These definitions are always kept up to date and in sync with the [catapult server](https://github.com/nemtech/catapult-server) code.

## Requirements

* Python >= 3.4

## Installation

1. Clone the ``catbuffer`` repository:

```bash
git clone https://github.com/nemtech/catbuffer
```

2. Install the package requirements:

```bash
pip3 install -r requirements.txt
```

## Usage

```bash
python3 main.py [OPTIONS]
```

| Option               | Description                                                                                                                                                | Default       |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------- |
| -s, --schema TEXT    | Input CATS file                                                                                                                                            |               |
| -o, --output TEXT    | Output directory                                                                                                                                           | _generated    |
| -i, --include TEXT   | Schema root directory                                                                                                                                      | ./schemas     |
| -g, --generator TEXT | Generator to use to produce output files (see the [available generators](https://github.com/nemtech/catbuffer-generators/blob/main/generators/All.py#L4)). |               |
| -c, --copyright TEXT | File containing copyright data to use with output files.                                                                                                   | ../HEADER.inc |

## Examples

In order to produce any output file, the [catbuffer-generators](https://github.com/nemtech/catbuffer-generators) project is needed. Please see this project's usage examples.

However, ``catbuffer`` can still be used on its own to parse input files and check their validity:

```bash
python3 main.py --schema schemas/transfer/transfer.cats
```

There is also a script in the ``scripts`` folder to parse and validate all schemas:

```bash
scripts/generate_all.sh
```

> **NOTE:**
> These scripts require Bash 4 or higher.

### Run the linter

```bash
pylint --load-plugins pylint_quotes main.py catparser test
pycodestyle --config=.pycodestyle .
```

### Run the tests

```bash
python3 -m unittest discover -v
```

Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
