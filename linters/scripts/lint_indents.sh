#!/bin/bash

set -ex

if ! [ -x "$(command -v rg)" ]; then
	echo "Error: rg is not installed"
	exit 1
fi

! rg \
		--files-with-matches \
		--type-not=json \
		--type-not=license \
		--type-not=markdown \
		--type-not=rst \
		--type-not=yaml \
		'^  ' "$(git rev-parse --show-toplevel)" \
	| grep -vE '\.eslintrc|testnet/summary\.txt|.git/hooks/.*\.sample'
