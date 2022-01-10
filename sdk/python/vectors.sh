#!/bin/bash

python3 -m tests.vectors.all --blockchain nem --vectors ../test-vectors/nem
python3 -m tests.vectors.all --blockchain symbol --vectors ../test-vectors/symbol
