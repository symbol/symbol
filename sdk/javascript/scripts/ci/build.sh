#!/bin/bash

set -ex

bash scripts/run_catbuffer_generator.sh dryrun

cd wasm
rustup default stable
wasm-pack build --release --target nodejs
cd ..

npm run bundle
