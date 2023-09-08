#!/bin/bash

set -ex

find . -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -type f -name "*.py" -print0 | xargs -0 git check-ignore -nv | grep :: | cut -f2- | \
	PYTHONPATH=. xargs python3 -m isort \
	--line-length 140 \
	--indent "	" \
	--multi-line 3 \
	--check-only
find . -name "nc" -prune -o -name "sc" -prune -o -type f -name "*.py" -print0 | xargs -0 git check-ignore -nv | grep :: | cut -f2- | \
	PYTHONPATH=. xargs python3 -m pycodestyle \
	--config="$(git rev-parse --show-toplevel)/linters/python/.pycodestyle"

# Cygwin is install in our Windows images so pick the correct separator if Windows(Msys)
SEPARATOR="$([ "$(uname -o)" = "Msys" ] && echo ";" || echo ":")"
find . -name "nc" -prune -o -name "sc" -prune -o -type f -name "*.py" -print0 | xargs -0 git check-ignore -nv | grep :: | cut -f2- | \
	PYTHONPATH=".${SEPARATOR}$(git rev-parse --show-toplevel)/catbuffer/parser" xargs python3 -m pylint \
	--rcfile "$(git rev-parse --show-toplevel)/linters/python/.pylintrc" \
	--load-plugins pylint_quotes

# generated code with some suppressions
find . -name "nc" -o -name "sc" -o -type f -name "*.py" -print0 | xargs -0 git check-ignore -nv | grep :: | cut -f2- | \
	PYTHONPATH=. xargs python3 -m pycodestyle \
	--config="$(git rev-parse --show-toplevel)/linters/python/.pycodestyle" \
	--max-line-length=215
find . -name "nc" -o -name "sc" -o -type f -name "*.py" -print0 | xargs -0 git check-ignore -nv | grep :: | cut -f2- | \
	PYTHONPATH=".${SEPARATOR}$(git rev-parse --show-toplevel)/catbuffer/parser" xargs python3 -m pylint \
	--rcfile "$(git rev-parse --show-toplevel)/linters/python/.pylintrc" \
	--load-plugins pylint_quotes \
	--disable=duplicate-code
