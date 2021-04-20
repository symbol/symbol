#!/bin/bash

find . -type f -name "*.sh" -print0 | xargs -0 shellcheck
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 "$(pyenv which isort)" --check-only --line-length 140
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 "$(pyenv which pycodestyle)" --config=.pycodestyle
find . -type f -name "*.py" -print0 | PYTHONPATH=. xargs -0 "$(pyenv which pylint)" --load-plugins pylint_quotes
