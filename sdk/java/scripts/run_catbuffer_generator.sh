#!/bin/bash
# Java catbuffer generator runner. Mirrors sdk/javascript/scripts/run_catbuffer_generator.sh.
#
# Generates Java model classes for each blockchain (nem, symbol) by invoking catparser with the
# `generator.Generator` class from `sdk/java/generator/`. Output goes to
# `sdk/java/src/main/java/org/symbol/sdk/<blockchain>/models/` (one file per type, in a dedicated
# subpackage that keeps the hand-written runtime classes such as Address, KeyPair, Network, ...
# in the parent `org.symbol.sdk.<blockchain>` package uncluttered).

set -ex

function generate_code() {
	# $1 blockchain (nem|symbol)
	# $2 destination subdirectory under sdk/java/src/main/java/org/symbol/sdk

	local git_root
	git_root="$(git rev-parse --show-toplevel)"

	PYTHONPATH="${git_root}/catbuffer/parser:${git_root}/sdk/java" python3 -m catparser \
		--schema "${git_root}/catbuffer/schemas/$1/all_generated.cats" \
		--include "${git_root}/catbuffer/schemas/$1" \
		--output "${git_root}/sdk/java/src/main/java/org/symbol/sdk/$2" \
		--quiet \
		--generator generator.Generator
}

if [[ $# -eq 0 ]]; then
	echo "updating generated code in git"
	for name in "nem" "symbol"; do
		models_dir="$(git rev-parse --show-toplevel)/sdk/java/src/main/java/org/symbol/sdk/${name}/models"
		# Remove the entire generated models/ subdirectory; hand-written files live one level
		# up in the parent package and are untouched.
		rm -rf "${models_dir}"
		generate_code "${name}" "${name}/models"
	done
	# The Python templates emit canonical-but-unwrapped Java; Spotless (eclipse formatter) owns
	# line-wrapping/whitespace, so format the freshly generated tree to match the committed style.
	# SKIP_SPOTLESS is set by the `gradle generateModels` task, which finalizes with spotlessApply
	# itself — running gradlew here too would nest one Gradle build inside another and deadlock.
	if [[ -z "${SKIP_SPOTLESS:-}" ]]; then
		# -PspotlessGenerated scopes the formatter to the generated subtrees only (see build.gradle.kts).
		(cd "$(git rev-parse --show-toplevel)/sdk/java" && ./gradlew --no-daemon spotlessApply -PspotlessGenerated)
	fi
elif [[ "$1" = "dryrun" ]]; then
	echo "running dryrun"
	for name in "nem" "symbol"; do
		generate_code "${name}" "${name}_dryrun/models"
		rm -rf "$(git rev-parse --show-toplevel)/sdk/java/src/main/java/org/symbol/sdk/${name}_dryrun"
	done
else
	echo "unknown options"
	exit 1
fi
