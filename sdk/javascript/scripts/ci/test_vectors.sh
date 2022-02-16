#!/bin/bash

set -ex

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "vectors:jenkins" || echo "vectors")

SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" npm run "cat${TEST_MODE}"

BLOCKCHAIN=nem npm run "${TEST_MODE}"
BLOCKCHAIN=symbol npm run "${TEST_MODE}"
