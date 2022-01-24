#!/bin/bash

set -ex

BLOCKCHAIN=nem npm run catvectors
BLOCKCHAIN=symbol npm run catvectors

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "vectors:jenkins" || echo "vectors")
BLOCKCHAIN=nem npm run "${TEST_MODE}"
BLOCKCHAIN=symbol npm run "${TEST_MODE}"
