#!/bin/bash

set -ex

function generate_examples_docs() {
	# $1 destination
	# $2 nonzero for dryrun

	local docs_folder
	docs_folder="$(git rev-parse --show-toplevel)/sdk/python/examples/docs"
	jinja2 "${docs_folder}/__main__.py.tmpl" > "${docs_folder}/$1"

	if [[ $2 -ne 0 ]]; then
		diff --strip-trailing-cr "${docs_folder}/__main__.py" "${docs_folder}/$1"
		rm "${docs_folder}/$1"
	fi
}

if [[ $# -eq 0 ]]; then
	echo "updating generated examples in git"
	generate_examples_docs "__main__.py" 0
elif [[ "$1" = "dryrun" ]]; then
	echo "running dryrun diff"
	generate_examples_docs "__main__2.py" 1
else
	echo "unknown options"
	exit 1
fi
