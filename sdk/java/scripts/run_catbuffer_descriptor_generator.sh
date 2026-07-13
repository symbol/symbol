#!/bin/bash
# Java catbuffer typed-descriptor generator runner. Mirrors
# sdk/javascript/scripts/run_catbuffer_generator_ts.sh.
#
# Generates Java typed-descriptor classes for each blockchain (nem, symbol) by invoking catparser
# with the `generator.DescriptorGenerator` class from `sdk/java/generator/` (same Python package
# as the model generator). Output goes to `sdk/java/src/main/java/org/symbol/sdk/<blockchain>/
# descriptors/` (a dedicated subpackage kept separate from the generated `models/` subpackage
# and the hand-written runtime classes).

set -ex

function generate_code() {
	# $1 blockchain (nem|symbol)
	# $2 destination subdirectory under sdk/java/src/main/java/org/symbol/sdk

	local git_root
	git_root="$(git rev-parse --show-toplevel)"

	PYTHONPATH="${git_root}/catbuffer/parser:${git_root}/sdk/java" python3 -m catparser \
		--schema "${git_root}/catbuffer/schemas/$1/all_transactions.cats" \
		--include "${git_root}/catbuffer/schemas/$1" \
		--output "${git_root}/sdk/java/src/main/java/org/symbol/sdk/$2" \
		--quiet \
		--generator generator.DescriptorGenerator
}

if [[ $# -eq 0 ]]; then
	echo "updating generated code in git"
	for name in "nem" "symbol"; do
		descriptors_dir="$(git rev-parse --show-toplevel)/sdk/java/src/main/java/org/symbol/sdk/${name}/descriptors"
		# Remove the entire generated descriptors/ subdirectory; hand-written files live one level
		# up in the parent package and are untouched.
		rm -rf "${descriptors_dir}"
		generate_code "${name}" "${name}/descriptors"
	done
	# The Python templates emit canonical-but-unwrapped Java; Spotless (eclipse formatter) owns
	# line-wrapping/whitespace, so format the freshly generated tree to match the committed style.
	# SKIP_SPOTLESS is set by the `gradle generateDescriptors` task, which finalizes with
	# spotlessApply itself — running gradlew here too would nest one Gradle build in another and deadlock.
	if [[ -z "${SKIP_SPOTLESS:-}" ]]; then
		# -PspotlessGenerated scopes the formatter to the generated subtrees only (see build.gradle.kts).
		(cd "$(git rev-parse --show-toplevel)/sdk/java" && ./gradlew --no-daemon spotlessApply -PspotlessGenerated)
	fi
elif [[ "$1" = "dryrun" ]]; then
	echo "running dryrun"
	for name in "nem" "symbol"; do
		generate_code "${name}" "${name}_descriptor_dryrun/descriptors"
		rm -rf "$(git rev-parse --show-toplevel)/sdk/java/src/main/java/org/symbol/sdk/${name}_descriptor_dryrun"
	done
else
	echo "unknown options"
	exit 1
fi
