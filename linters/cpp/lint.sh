#!/bin/bash

set -ex

find . -type f -iname '*.h' -o -iname '*.cpp' | xargs clang-format \
	--style="file:$(git rev-parse --show-toplevel)/linters/cpp/.clang-format" \
	--Werror \
	--dry-run
