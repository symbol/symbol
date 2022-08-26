#!/bin/bash

set -ex

cd wasm
rustup default stable
cargo run --release --example all -- --blockchain nem --vectors "$(git rev-parse --show-toplevel)/tests/vectors/nem/crypto"
cargo run --release --example all -- --blockchain symbol --vectors "$(git rev-parse --show-toplevel)/tests/vectors/symbol/crypto"
cd ..

TEST_MODE=$([ "$1" = "code-coverage" ] && echo "vectors:jenkins" || echo "vectors")

SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" npm run "cat${TEST_MODE}"

BLOCKCHAIN=nem npm run "${TEST_MODE}"
BLOCKCHAIN=symbol npm run "${TEST_MODE}"
