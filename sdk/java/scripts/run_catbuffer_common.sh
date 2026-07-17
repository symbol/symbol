# shellcheck shell=bash
# Shared runner logic for the Java catbuffer generators. Sourced by run_catbuffer_generator.sh (models) and
# run_catbuffer_descriptor_generator.sh (descriptors); those two differ only in the six CATBUFFER_* config
# values the wrapper sets before sourcing this file:
#   CATBUFFER_SCHEMA           schema file under catbuffer/schemas/<name>/ (all_generated.cats | all_transactions.cats)
#   CATBUFFER_ARTIFACT         generated subpackage folder (models | descriptors)
#   CATBUFFER_DRYRUN_SUFFIX    dryrun sibling-folder suffix (_dryrun | _descriptor_dryrun)
#   CATBUFFER_GENERATOR        default catparser generator class
#   CATBUFFER_SWEEP_GENERATOR  sweep-test generator class
#   CATBUFFER_SWEEP_FILE       generated sweep-test filename
#
# shellcheck disable=SC2154  # the CATBUFFER_* config vars are provided by the sourcing wrapper

set -ex

function generate_code() {
	# $1 blockchain (nem|symbol)
	# $2 destination subdirectory under sdk/java/src/main/java/org/symbol/sdk
	# $3 catparser generator class (default ${CATBUFFER_GENERATOR}; the sweep test passes its own)

	local git_root
	git_root="$(git rev-parse --show-toplevel)"

	PYTHONPATH="${git_root}/catbuffer/parser:${git_root}/sdk/java" python3 -m catparser \
		--schema "${git_root}/catbuffer/schemas/$1/${CATBUFFER_SCHEMA}" \
		--include "${git_root}/catbuffer/schemas/$1" \
		--output "${git_root}/sdk/java/src/main/java/org/symbol/sdk/$2" \
		--quiet \
		--generator "${3:-${CATBUFFER_GENERATOR}}"
}

function _cleanup_dryrun() {
	# Remove the throwaway dryrun trees. Run from an EXIT trap so a failed generate/format step cannot leak
	# _dryrun sibling packages into the working tree (where a later git add -A would sweep them into the repo).
	local git_root
	git_root="$(git rev-parse --show-toplevel)"
	# CATBUFFER_DRYRUN_SUFFIX is asserted non-empty in run_catbuffer, so these cannot collapse to a real dir.
	# shellcheck disable=SC2115
	rm -rf \
		"${git_root}/sdk/java/src/main/java/org/symbol/sdk/nem${CATBUFFER_DRYRUN_SUFFIX}" \
		"${git_root}/sdk/java/src/main/java/org/symbol/sdk/symbol${CATBUFFER_DRYRUN_SUFFIX}" \
		"${git_root}/sdk/java/src/test/java/org/symbol/sdk/nem${CATBUFFER_DRYRUN_SUFFIX}" \
		"${git_root}/sdk/java/src/test/java/org/symbol/sdk/symbol${CATBUFFER_DRYRUN_SUFFIX}"
}

function run_catbuffer() {
	# Fail loudly if a wrapper omitted any config: an empty CATBUFFER_ARTIFACT / CATBUFFER_DRYRUN_SUFFIX would
	# make an rm -rf below target a real source directory instead of a generated/dryrun one.
	: "${CATBUFFER_SCHEMA:?}" "${CATBUFFER_ARTIFACT:?}" "${CATBUFFER_DRYRUN_SUFFIX:?}" \
		"${CATBUFFER_GENERATOR:?}" "${CATBUFFER_SWEEP_GENERATOR:?}" "${CATBUFFER_SWEEP_FILE:?}"

	if [[ $# -eq 0 ]]; then
		echo "updating generated code in git"
		for name in "nem" "symbol"; do
			local target_dir
			target_dir="$(git rev-parse --show-toplevel)/sdk/java/src/main/java/org/symbol/sdk/${name}/${CATBUFFER_ARTIFACT}"
			# Remove the entire generated subdirectory; hand-written files live one level up in the parent
			# package and are untouched.
			rm -rf "${target_dir}"
			generate_code "${name}" "${name}/${CATBUFFER_ARTIFACT}"

			# emit the per-type sweep test (into the mirrored test tree) via its own generator
			generate_code "${name}" "${name}/${CATBUFFER_ARTIFACT}" "${CATBUFFER_SWEEP_GENERATOR}"
		done
		# The Python templates emit canonical-but-unwrapped Java; Spotless (eclipse formatter) owns
		# line-wrapping/whitespace, so format the freshly generated tree to match the committed style.
		# SKIP_SPOTLESS is set by the `gradle generateModels` / `generateDescriptors` task, which finalizes
		# with spotlessApply itself — running gradlew here too would nest one Gradle build in another and deadlock.
		if [[ -z "${SKIP_SPOTLESS:-}" ]]; then
			# -PspotlessGeneratedOnly scopes the formatter to the generated subtrees only (see build.gradle.kts).
			(cd "$(git rev-parse --show-toplevel)/sdk/java" && ./gradlew --no-daemon spotlessApply -PspotlessGeneratedOnly)
		fi
	elif [[ "$1" = "dryrun" ]]; then
		echo "running dryrun diff"
		local git_root base test_base
		git_root="$(git rev-parse --show-toplevel)"
		base="${git_root}/sdk/java/src/main/java/org/symbol/sdk"
		test_base="${git_root}/sdk/java/src/test/java/org/symbol/sdk"
		# clean up the dryrun trees on any exit (success, diff mismatch, or a mid-run generate/format failure)
		trap _cleanup_dryrun EXIT
		for name in "nem" "symbol"; do
			generate_code "${name}" "${name}${CATBUFFER_DRYRUN_SUFFIX}/${CATBUFFER_ARTIFACT}"

			generate_code "${name}" "${name}${CATBUFFER_DRYRUN_SUFFIX}/${CATBUFFER_ARTIFACT}" "${CATBUFFER_SWEEP_GENERATOR}"
		done
		# Format the dryrun trees like the committed files; the -PspotlessGeneratedOnly glob
		# (org/symbol/sdk/*/{models,descriptors}) also matches the dryrun sibling dirs.
		(cd "${git_root}/sdk/java" && ./gradlew --no-daemon spotlessApply -PspotlessGeneratedOnly)

		for name in "nem" "symbol"; do
			diff --strip-trailing-cr -r -I "package org.symbol.sdk.${name}" \
				"${base}/${name}/${CATBUFFER_ARTIFACT}" "${base}/${name}${CATBUFFER_DRYRUN_SUFFIX}/${CATBUFFER_ARTIFACT}"

			diff --strip-trailing-cr -I "package org.symbol.sdk.${name}" \
				"${test_base}/${name}/${CATBUFFER_ARTIFACT}/${CATBUFFER_SWEEP_FILE}" \
				"${test_base}/${name}${CATBUFFER_DRYRUN_SUFFIX}/${CATBUFFER_ARTIFACT}/${CATBUFFER_SWEEP_FILE}"
		done
	else
		echo "unknown options"
		exit 1
	fi
}
