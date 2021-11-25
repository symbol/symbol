#!/bin/bash

set -ex

find . -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -name "sc" -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m isort --check-only --line-length 140
find . -name "sc" -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pycodestyle --config=.pycodestyle
find . -name "sc" -prune -o -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 python3 -m pylint --load-plugins pylint_quotes
