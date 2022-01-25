#!/bin/bash

set -ex

bash scripts/ci/lint_python.sh
bash scripts/ci/lint_cpp.sh
