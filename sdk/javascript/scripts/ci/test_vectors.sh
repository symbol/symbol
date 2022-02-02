#!/bin/bash

set -ex

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "coverage:vectors" || echo "vectors")
BLOCKCHAIN=nem npm run "${TEST_MODE}"
BLOCKCHAIN=symbol npm run "${TEST_MODE}"
