#!/bin/bash

set -ex

LINT_MODE=$([ -z "${JENKINS_HOME}" ] && echo "lint" || echo "lint:jenkins")
npm run "${LINT_MODE}"
bash scripts/ci/lint_python.sh
