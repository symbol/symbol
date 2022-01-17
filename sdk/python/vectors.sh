#!/bin/bash

PYTHONPATH=. SCHEMAS_PATH="../../catbuffer/schemas/test-vectors" pytest tests/generator

python3 -m tests.vectors.all --blockchain nem --vectors ../test-vectors/nem
python3 -m tests.vectors.all --blockchain symbol --vectors ../test-vectors/symbol
