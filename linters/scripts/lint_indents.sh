#!/bin/bash

set -ex

! rg \
		--files-with-matches \
		--type-not=json \
		--type-not=license \
		--type-not=markdown \
		--type-not=rst \
		--type-not=yaml \
		'^  ' "$(git rev-parse --show-toplevel)" \
	| grep -vE '\.eslintrc|testnet/summary\.txt|.git/hooks/.*\.sample'
