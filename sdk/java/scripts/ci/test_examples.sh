#!/bin/bash
# Run each ported Java example end-to-end. Mirrors sdk/javascript/scripts/ci/test_examples.sh.
# Invokes Gradle once per example so that a failure isolates to the specific task.

set -ex

cd "$(git rev-parse --show-toplevel)/sdk/java"

GRADLE_ARGS=("--no-daemon" "-q")
if [ "$1" = "code-coverage" ]; then
	GRADLE_ARGS+=("-Pcoverage")
fi

./gradlew "${GRADLE_ARGS[@]}" :examples:runBip32Keypair
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionAggregate
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionMultisig
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionSignNem
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionSignSymbol

./gradlew "${GRADLE_ARGS[@]}" :examples:runReadmeNem
./gradlew "${GRADLE_ARGS[@]}" :examples:runReadmeSymbol
