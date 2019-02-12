# catbuffer

[![Build Status](https://api.travis-ci.org/nemtech/catbuffer.svg?branch=master)](https://travis-ci.org/nemtech/catbuffer)

Catbuffer library defines the protocol to serialize and deserialize Catapult entities. SDKs and applications use the generated code to interact with REST transaction endpoint.

## Supported languages

- C++

## Requirements

* Python >= 3.4
* pip install -r requirements.txt

## Usage

python main.py [OPTIONS]

| Option               | Description                                             | Default       |
|----------------------|---------------------------------------------------------|---------------|
| -s, --schema TEXT    | input CATS file                                         |               |
| -o, --output TEXT    | output directory                                        | _generated    |
| -i, --include TEXT   | schema root directory                                   | ./schemas     |
| -g, --generator TEXT | generator to use to produce output files                |               |
| -c, --copyright TEXT | file containing copyright data to use with output files | ../HEADER.inc |


## Examples

### Generate transaction builders

The [schemas](schemas) define the entities data structure. The library generates the leanest code necessary to serialize and deserialize defined entities.

 For example, run the following command to generate C++ transaction builders for a transfer transaction:

```
python main.py --schema schemas/transfer/transfer.cats --generator cpp_builder
```

The generator creates a new file under ``_generated/cpp_builder`` folder.

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
