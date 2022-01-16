#!/bin/bash

set -ex

find . -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -name "nc" -prune -o -name "sc" -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m isort --line-length 140 --indent "	" --multi-line 3 --check-only
find . -name "nc" -prune -o -name "sc" -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pycodestyle --config=.pycodestyle
find . -name "nc" -prune -o -name "sc" -prune -o -type f -name "*.py" -print0 | PYTHONPATH=".:$(git rev-parse --show-toplevel)/catbuffer/parser" xargs -0 python3 -m pylint --load-plugins pylint_quotes
