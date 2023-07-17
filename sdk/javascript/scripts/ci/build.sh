#!/bin/bash

set -ex

bash scripts/run_catbuffer_generator.sh dryrun

# build wasm variants
cd wasm
rustup default stable
wasm-pack build --release --no-typescript --target nodejs --out-dir ../_build/wasm/node
wasm-pack build --release --no-typescript --target web --out-dir ../_build/wasm/web
cd ..

# build frontend webpack
npm run bundle

# build TS bindings
npx tsc -p ./tsconfig-generate.json
npx tsc
find src -name "*.ts" -type f -delete
find test -name "*.ts" -type f -delete
find vectors -name "*.ts" -type f -delete
