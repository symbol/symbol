#!/bin/bash

set -ex

bash scripts/ci/lint_python.sh

npm-groovy-lint -c "$(git rev-parse --show-toplevel)/linters/groovy/.groovylintrc.json"
