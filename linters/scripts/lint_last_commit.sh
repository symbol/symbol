#!/bin/bash

set -ex

GITLINT_MODULES="$(cat "$(git rev-parse --show-toplevel)"/.gitlintmodules)"
GITLINT_CATEGORIES="feat|bug|fix|build|perf|task"
gitlint \
	-C "$(git rev-parse --show-toplevel)/linters/git/.gitlint" \
	-c title-match-regex.regex="^\\[(${GITLINT_MODULES})\\]( (${GITLINT_CATEGORIES}))?: [\S ]+[a-zA-Z0-9]$"
