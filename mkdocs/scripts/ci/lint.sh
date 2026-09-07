#!/bin/bash

set -ex

npm run lint
../sdk/java/gradlew --no-daemon spotlessCheck checkJavaSnippetLineLength
bash scripts/ci/lint_python.sh
