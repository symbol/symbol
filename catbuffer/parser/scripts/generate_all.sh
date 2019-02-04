#!/bin/bash

if [ "$#" -lt 1 ]; then
	echo "usage: script <builder> <nis2_root>"
	exit 1
fi

builder="$1"

function generate_all {
	local start_success_color="\033[1;34m"
	local start_error_color="\033[1;31m"
	local end_color="\033[0m"

	local inputs=(
		"accountlink/accountlink"
		"lock_hash/hashlock"
		"lock_secret/secretlock"
		"lock_secret/secretproof"
		"mosaic/mosaicdefinition"
		"mosaic/mosaicsupplychange"
		"multisig/modifymultisigaccount"
		"namespace/addressalias"
		"namespace/mosaicalias"
		"namespace/registernamespace"
		"property/addressproperty"
		"property/mosaicproperty"
		"property/transactiontypeproperty"
		"transfer/transfer"
	)

	for input in ${inputs[*]}
	do
		echo "generating ${input}"
		python3 main.py --schema ./schemas/${input}.cats --output _generated --generator ${builder} --copyright $1
		if [ $? -ne 0 ]; then
			echo "${start_error_color}ERROR: failed generating ${input}${end_color}"
			exit 1
		fi
	done

	echo "${start_success_color}SUCCESS: generation complete with no errors${end_color}"
}

if [ "$#" -lt 2 ]; then
	generate_all "../HEADER.inc"
else
	nis2_root="$2"
	rm -rf _generated/${builder}
	generate_all "${nis2_root}/HEADER.inc"
	cp ./_generated/${builder}/* ${nis2_root}/sdk/src/builders/
fi
