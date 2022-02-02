#!/bin/bash

set -ex

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "coverage:test" || echo "test")
npm run "${TEST_MODE}"
