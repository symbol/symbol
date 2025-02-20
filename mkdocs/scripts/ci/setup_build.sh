set -ex

npm install

pushd ../sdk/javascript
npm install
npx tsc -p ./tsconfig/build-bindings.json
popd
