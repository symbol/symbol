#!/bin/bash

set -ex

# Install Typedoc
npm install

# Build Typescript SDK
pushd ../sdk/javascript
npm install
npx tsc -p ./tsconfig/build-bindings.json
popd

# Build OpenAPI spec
pushd ../openapi
npm install
npm run build:openapi-yaml
popd
