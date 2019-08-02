#!/bin/bash

function generate_batch {
	local start_success_color="\033[1;34m"
	local start_error_color="\033[1;31m"
	local end_color="\033[0m"

	local inputs=$1
	for input in ${inputs[*]}
	do
		echo "generating ${input}"
		if [ "$#" -lt 2 ]; then
			python3 main.py --schema ./schemas/${input}.cats
		else
			python3 main.py --schema ./schemas/${input}.cats --output _generated --generator $2 --copyright $3
		fi

		if [ $? -ne 0 ]; then
			echo "${start_error_color}ERROR: failed generating ${input}${end_color}"
			exit 1
		fi
	done

	echo "${start_success_color}SUCCESS: generation complete with no errors${end_color}"
}
