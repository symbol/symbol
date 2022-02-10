#!/bin/bash

set -ex

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "vectors:jenkins" || echo "vectors")

BLOCKCHAIN=nem npm run "cat${TEST_MODE}"
BLOCKCHAIN=symbol npm run "cat${TEST_MODE}"

BLOCKCHAIN=nem npm run "${TEST_MODE}"
BLOCKCHAIN=symbol npm run "${TEST_MODE}"
