set -ex

pushd ../sdk/javascript
npx tsc -p ./tsconfig/build-bindings.json
popd
