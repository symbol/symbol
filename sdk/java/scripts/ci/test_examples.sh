#!/bin/bash

set -ex

GRADLE_ARGS=("--no-daemon" "-q")
if [ "$1" = "code-coverage" ]; then
	GRADLE_ARGS+=("-Pcoverage")
fi

./gradlew "${GRADLE_ARGS[@]}" :examples:runBip32Keypair
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionAggregate --args="--private src/main/resources/zero.sha256.txt"
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionMultisig
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionSign --args="--blockchain=nem"
./gradlew "${GRADLE_ARGS[@]}" :examples:runTransactionSign --args="--blockchain=symbol"

./gradlew "${GRADLE_ARGS[@]}" :examples:runReadmeNem
./gradlew "${GRADLE_ARGS[@]}" :examples:runReadmeSymbol
