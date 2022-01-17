#!/bin/bash

set -ex

PYTHONPATH=. SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" pytest tests/generator

python3 -m tests.vectors.all --blockchain nem --vectors "$(git rev-parse --show-toplevel)/tests/vectors/nem/crypto"
python3 -m tests.vectors.all --blockchain symbol --vectors "$(git rev-parse --show-toplevel)/tests/vectors/symbol/crypto"
