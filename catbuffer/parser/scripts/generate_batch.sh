#!/bin/bash

function generate_batch {
	local start_success_color="\033[1;34m"
	local start_error_color="\033[1;31m"
	local end_color="\033[0m"

	local -n inputs=$1
	local folder="$2"
	local counter=0
	for input in ${inputs[@]};
	do
		echo "generating ${input}"
		python_args=(
			"${folder}/main.py"
			--schema "${folder}/schemas/${input}.cats"
			--include "${folder}/schemas")
		if [ "$#" -ge 3 ]; then
			python_args+=(
				--output "${folder}/_generated/$3"
				--generator "$3"
				--copyright "${folder}/HEADER.inc")
		fi

		python3 "${python_args[@]}"

		if [ $? -ne 0 ]; then
			echo -e "${start_error_color}ERROR: failed generating ${input}${end_color}"
			exit 1
		fi

		((++counter))
	done

	echo -e "${start_success_color}SUCCESS: generation complete with no errors [${counter} files processed]${end_color}"
}
