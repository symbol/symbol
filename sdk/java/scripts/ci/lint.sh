#!/bin/bash

set -ex

./gradlew --no-daemon spotlessCheck
bash scripts/ci/lint_python.sh
