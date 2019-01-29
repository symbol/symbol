# catbuffer

[![Build Status](https://api.travis-ci.org/nemtech/catbuffer.svg?branch=master)](https://travis-ci.org/nemtech/catbuffer)

Catbuffer library defines the protocol to serialize and deserialize Catapult entities. The library comes with code generators for different languages. SDKs and applications use the generated code to interact with REST transaction endpoint.

Check the complete [serialization documentation](https://nemtech.github.io/api/serialization.html).

## Parse an schema and generate transaction builders

A schema file defines the entity data structure. The library generates the leanest code necessary to serialize and deserialize defined entities.

Generate the code for a determined schema in one of the available languages. For example, run the following command to generate C++ code to serialize and deserialize a transfer transaction:

```
python3 main.py -i schemas/transfer.cats -g cpp_builder
```
The generator creates a new file under _generated/cpp folder. Repeat the process using a different input schema (-i) or generator (-g) as needed.

## Run lint
```
pylint --load-plugins pylint_quotes main.py catparser generators test
pycodestyle --config=.pycodestyle .
```

## Run tests
```
python3 -m unittest discover -v
```

Copyright (c) 2018 Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp Licensed under the [MIT License](LICENSE)
