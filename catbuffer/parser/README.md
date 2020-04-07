# catbuffer

[![Build Status](https://api.travis-ci.com/nemtech/catbuffer.svg?branch=master)](https://travis-ci.com/nemtech/catbuffer)

The catbuffer library defines the protocol to serialize and deserialize Catapult entities.

In combination with the [catbuffer-generators](https://github.com/nemtech/catbuffer-generators) project, developers can generate builder classes for a given set of programming languages. For example, the [NEM2-SDK](https://nemtech.github.io/sdk) uses the generated code to operate with the entities in binary form before announcing them to the network.

## Requirements

* Python >= 3.4

## Installation

1. Clone the ``catbuffer-generators`` repository.

``git clone https://github.com/nemtech/catbuffer-generators``

2. Install the package requirements.

``pip install -r requirements.txt``

3. Clone the ``catbuffer`` repository inside the ``catbuffer-generators`` folder.

``git clone https://github.com/nemtech/catbuffer``

## Usage

python main.py [OPTIONS]

| Option               | Description                                             | Default       |
|----------------------|---------------------------------------------------------|---------------|
| -s, --schema TEXT    | input CATS file                                         |               |
| -o, --output TEXT    | output directory                                        | _generated    |
| -i, --include TEXT   | schema root directory                                   | ./schemas     |
| -g, --generator TEXT | generator to use to produce output files. See the [available generators](https://github.com/nemtech/catbuffer-generators/blob/master/generators/All.py#L4).|               |
| -c, --copyright TEXT | file containing copyright data to use with output files | ../HEADER.inc |

## Generate transaction builders

The [schemas](schemas) folder defines the entities' data structure. With the help of a code generator, you can produce the leanest code necessary to serialize and deserialize those entities.

For example, run the following command to generate C++ transaction builders for a [TransferTransaction](https://nemtech.github.io/concepts/transfer-transaction.html#transfertransaction):

```
python main.py --schema schemas/transfer/transfer.cats --generator cpp_builder
```
You can also generate code for all the schemas running the following command under the ``catbuffer-generators`` directory:

```
python ../scripts/generate_all.sh cpp_builder
```

The generator creates a new file for every schema under the ``catbuffer/_generated/cpp_builder`` folder.

### Run the linter
```
pylint --load-plugins pylint_quotes main.py catparser generators test
pycodestyle --config=.pycodestyle .
```

### Run the tests
```
python -m unittest discover -v
```

Copyright (c) 2018 Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp Licensed under the [MIT License](LICENSE)
