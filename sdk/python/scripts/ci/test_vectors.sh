#!/bin/bash

set -ex

PYTHONPATH=. SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" coverage run --append -m pytest tests/generator

coverage run --append -m tests.vectors.all --blockchain nem --vectors "$(git rev-parse --show-toplevel)/tests/vectors/nem/crypto"
coverage run --append -m tests.vectors.all --blockchain symbol --vectors "$(git rev-parse --show-toplevel)/tests/vectors/symbol/crypto"
