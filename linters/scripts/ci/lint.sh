#!/bin/bash

set -ex

bash scripts/ci/lint_python.sh "consider-using-f-string"

"$(git rev-parse --show-toplevel)/linters/scripts/lint_indents.sh"
"$(git rev-parse --show-toplevel)/linters/scripts/lint_yaml.sh"
