#!/bin/bash

set -ex

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "test:jenkins" || echo "test")
TEST_SCRIPT='test' npm run "${TEST_MODE}"
