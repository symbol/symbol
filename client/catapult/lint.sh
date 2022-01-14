#!/bin/bash

set -ex

catlint() {
	local cpp_linters_directory
	cpp_linters_directory="$(git rev-parse --show-toplevel)/linters/cpp"

	PYTHONPATH="$cpp_linters_directory" python3 $"$cpp_linters_directory"/checkProjectStructure.py --text \
		--dest-dir . \
		--dep-check-dir src \
		--dep-check-dir extensions \
		--dep-check-dir plugins

	local -i lint_error_code=$?
	echo "catlint completed with code ${lint_error_code}"

	return $((lint_error_code & 0xFF))
}

catlint
