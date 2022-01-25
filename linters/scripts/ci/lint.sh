#!/bin/bash

set -ex

bash scripts/ci/lint_python.sh

"$(git rev-parse --show-toplevel)/linters/scripts/find_space_indents.sh"
"$(git rev-parse --show-toplevel)/linters/scripts/run_yamllint.sh"
