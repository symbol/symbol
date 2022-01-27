#!/bin/bash

set -ex

# prepare toml (add requirements)
while IFS= read -r line; do poetry add "$line"; done < requirements.txt
cat pyproject.toml

# build package
poetry build

# publish package
if [ "$1" == "release" ]; then
	poetry config "pypi-token.pypi" "${POETRY_PYPI_TOKEN_PYPI}"
	poetry publish
else
	poetry config "repositories.test" "https://test.pypi.org/legacy/"
	poetry config "pypi-token.test" "${POETRY_PYPI_TOKEN_PYPI}"
	poetry publish --repository "test"
fi
