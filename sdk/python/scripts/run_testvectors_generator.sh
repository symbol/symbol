#!/bin/bash

set -ex

function generate_vectors() {
	# $1 destination

	mkdir -p "$(git rev-parse --show-toplevel)/tests/$1"
	python3 -m testvectors --output "$(git rev-parse --show-toplevel)/tests/$1"
}

function diff_vectors() {
	# $1 blockchain
	# $2 entity_type

	diff --strip-trailing-cr \
		"$(git rev-parse --show-toplevel)/tests/vectors/$1/models/$2.json" \
		"$(git rev-parse --show-toplevel)/tests/vectors2/$1/models/$2.json"
}

if [[ $# -eq 0 ]]; then
	echo "updating generated vectors in git"
	generate_vectors "vectors"
elif [[ "$1" = "dryrun" ]]; then
	echo "running dryrun diff"
	generate_vectors "vectors2"

	for blockchain in "nem" "symbol";
	do
		diff_vectors "${blockchain}" "transactions"
	done

	for entity_type in "blocks" "receipts" "other";
	do
		diff_vectors "symbol" "${entity_type}"
	done

	rm -rf "$(git rev-parse --show-toplevel)/tests/vectors2"
else
	echo "unknown options"
	exit 1
fi
