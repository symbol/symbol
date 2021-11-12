#!/bin/bash

set -ex

find . -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 "$(command -v isort)" --check-only --line-length 140
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 "$(command -v pycodestyle)" --config=.pycodestyle
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 "$(command -v pylint)" --load-plugins pylint_quotes
