#!/bin/bash

set -ex

GRADLE_ARGS=("--no-daemon")
if [ "$1" = "code-coverage" ]; then
	GRADLE_ARGS+=("-Pcoverage")
fi

./gradlew "${GRADLE_ARGS[@]}" test
