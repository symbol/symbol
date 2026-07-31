#!/bin/bash

set -ex

GRADLE_ARGS=("--no-daemon")
if [ "$1" = "code-coverage" ]; then
	GRADLE_ARGS+=("-Pcoverage")
fi

SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" ./gradlew "${GRADLE_ARGS[@]}" catVectors

BLOCKCHAIN=nem ./gradlew "${GRADLE_ARGS[@]}" vectors
BLOCKCHAIN=symbol ./gradlew "${GRADLE_ARGS[@]}" vectors

if [ "$1" = "code-coverage" ]; then
	# render the combined coverage report
	# has written its .exec files into build/jacoco
	./gradlew "${GRADLE_ARGS[@]}" jacocoTestReport
fi
