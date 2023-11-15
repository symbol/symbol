#!/bin/bash

set -ex

function generate_vectors() {
	# $1 destination

	mkdir -p "$(git rev-parse --show-toplevel)/tests/$1"
	python3 -m testvectors --output "$(git rev-parse --show-toplevel)/tests/$1"
}

if [[ $# -eq 0 ]]; then
	echo "updating generated vectors in git"
	generate_vectors "vectors"
elif [[ "$1" = "dryrun" ]]; then
	echo "running dryrun diff"
	generate_vectors "vectors2"

	for name in "nem" "symbol";
	do
		diff --strip-trailing-cr \
			"$(git rev-parse --show-toplevel)/tests/vectors/${name}/models/transactions.json" \
			"$(git rev-parse --show-toplevel)/tests/vectors2/${name}/models/transactions.json"
	done

	rm -rf "$(git rev-parse --show-toplevel)/tests/vectors2"
else
	echo "unknown options"
	exit 1
fi
