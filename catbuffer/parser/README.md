# catbuffer

[![Build Status](https://api.travis-ci.org/nemtech/catbuffer.svg?branch=master)](https://travis-ci.org/nemtech/catbuffer)

catapult's data interchange format

## parse transfer schema
```
python3 main.py -i schemas/transfer.cats
```

## parse transfer schema and generate catapult cpp transaction builders
```
python3 main.py -i schemas/transfer.cats -o _generated -g cpp-builder
```

## run lint
```
pylint --load-plugins pylint_quotes main.py catparser generators test
pycodestyle --config=.pycodestyle .
```

## run tests
```
python3 -m unittest discover -v
```

Copyright (c) 2018 Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp Licensed under the [MIT License](LICENSE)
