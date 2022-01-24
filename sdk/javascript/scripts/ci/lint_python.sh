#!/bin/bash

set -ex

find . -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m isort \
	--line-length 140 \
	--indent "	" \
	--multi-line 3 \
	--check-only
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pycodestyle \
	--config="$(git rev-parse --show-toplevel)/linters/python/.pycodestyle"
find . -type f -name "*.py" -print0 | PYTHONPATH=".:$(git rev-parse --show-toplevel)/catbuffer/parser" xargs -0 python3 -m pylint \
	--rcfile "$(git rev-parse --show-toplevel)/linters/python/.pylintrc" \
	--load-plugins pylint_quotes
