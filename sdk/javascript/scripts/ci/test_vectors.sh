#!/bin/bash

set -ex

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "test:jenkins" || echo "vectors")
export TEST_SCRIPT='vectors'
BLOCKCHAIN=nem npm run "${TEST_MODE}"
BLOCKCHAIN=symbol npm run "${TEST_MODE}"
