#!/bin/bash

set -ex

find . \( -name node_modules -o -name .venv \) -prune -o -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . \( -name node_modules -o -name .venv \) -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m isort \
	--line-length 75 \
	--indent "	" \
	--multi-line 3 \
	--check-only
find . \( -name node_modules -o -name .venv \) -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pycodestyle \
	--config="$(git rev-parse --show-toplevel)/linters/python/.pycodestyle"
find . \( -name node_modules -o -name .venv \) -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pylint \
	--rcfile "$(git rev-parse --show-toplevel)/linters/python/.pylintrc"
