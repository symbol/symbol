#!/bin/bash

set -ex

TEST_RUNNER=$([ "$1" = "code-coverage" ] && echo "coverage run --append" || echo "python3")

function parse_schemas() {
	# $1 blockchain

	local git_root
	git_root="$(git rev-parse --show-toplevel)"

	PYTHONPATH="${git_root}/catbuffer/parser" ${TEST_RUNNER} -m catparser \
		--schema "${git_root}/catbuffer/schemas/$1/all.cats"  \
		--include "${git_root}/catbuffer/schemas/$1"
}

parse_schemas "nem"
parse_schemas "symbol"
