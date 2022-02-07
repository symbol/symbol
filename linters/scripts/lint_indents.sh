#!/bin/bash

set -ex

! rg \
		--files-with-matches \
		--type-not=markdown \
		--type-not=json \
		--type-not=yaml \
		--type-not=license \
		'^  ' "$(git rev-parse --show-toplevel)" \
	| grep -vE '\.eslintrc|testnet/summary\.txt|.git/hooks/.*\.sample'
