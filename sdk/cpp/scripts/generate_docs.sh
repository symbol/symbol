#!/bin/bash

set -ex

pushd .
cd "$(git rev-parse --show-toplevel)/sdk/python/docs"

sphinx-apidoc -o source ../symbolchain
make html

popd
