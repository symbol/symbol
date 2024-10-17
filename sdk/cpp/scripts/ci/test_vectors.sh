#!/bin/bash

set -ex

TEST_RUNNER=$([ "$1" = "code-coverage" ] && echo "coverage run --append" || echo "python3")

PYTHONPATH=. SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" ${TEST_RUNNER} -m pytest tests/vectors/catbuffer.py

${TEST_RUNNER} -m tests.vectors.all --blockchain nem --vectors "$(git rev-parse --show-toplevel)/tests/vectors/nem/crypto"
${TEST_RUNNER} -m tests.vectors.all --blockchain symbol --vectors "$(git rev-parse --show-toplevel)/tests/vectors/symbol/crypto"
