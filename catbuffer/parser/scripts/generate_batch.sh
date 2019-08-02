#!/bin/bash

function generate_batch {
	local start_success_color="\033[1;34m"
	local start_error_color="\033[1;31m"
	local end_color="\033[0m"

	local inputs=$1
	local folder="$2"
	for input in ${inputs[*]}
	do
		echo "generating ${input}"
		python_args=(${folder}/main.py)
		python_args+=(--schema ${folder}/schemas/${input}.cats)
		python_args+=(--include ${folder}/schemas)
		if [ "$#" -ge 3 ]; then
			python_args+=(--output ${folder}/_generated)
			python_args+=(--generator "$3")
			python_args+=(--copyright "${folder}/HEADER.inc")
		fi

		python3 "${python_args[@]}"

		if [ $? -ne 0 ]; then
			echo "${start_error_color}ERROR: failed generating ${input}${end_color}"
			exit 1
		fi
	done

	echo "${start_success_color}SUCCESS: generation complete with no errors${end_color}"
}
