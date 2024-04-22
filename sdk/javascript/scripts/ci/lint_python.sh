#!/bin/bash

set -ex

find . -name node_modules -prune -o -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -name node_modules -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m isort \
	--line-length 140 \
	--indent "	" \
	--multi-line 3 \
	--check-only
find . -name node_modules -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pycodestyle \
	--config="$(git rev-parse --show-toplevel)/linters/python/.pycodestyle"

# Cygwin is install in our Windows images so pick the correct separator if Windows(Msys)
SEPARATOR="$([ "$(uname -o)" = "Msys" ] && echo ";" || echo ":")"
find . -name node_modules -prune -o -type f -name "*.py" -print0 | \
	PYTHONPATH=".${SEPARATOR}$(git rev-parse --show-toplevel)/catbuffer/parser" xargs -0 python3 -m pylint \
	--rcfile "$(git rev-parse --show-toplevel)/linters/python/.pylintrc"
