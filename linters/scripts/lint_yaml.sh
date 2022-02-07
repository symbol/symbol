#!/bin/bash

set -ex

git ls-files "$(git rev-parse --show-toplevel)" \
	| grep -E '\.(yaml|yml)' \
	| xargs python3 -m yamllint -c "$(git rev-parse --show-toplevel)/linters/yaml/.yamllint"
