#!/bin/bash

set -ex

architecture="$1"
git_root_directory="$(git rev-parse --show-toplevel)"
scripts_directory="${git_root_directory}/jenkins/catapult"
build_configuration="${scripts_directory}/configurations/tests-conan.yaml"
compiler_configuration="${scripts_directory}/configurations/${architecture}/gcc-latest.yaml"
if [ -z "${JENKINS_HOME}" ]
then
	python3 "${scripts_directory}/runDockerBuild.py" \
		--compiler-configuration "${compiler_configuration}" \
		--build-configuration "${build_configuration}" \
		--operating-system ubuntu \
		--user "$(id -u):$(id -g)" \
		--destination-image-label gcc-latest-local \
		--source-path "${git_root_directory}"
else
	output_path="${git_root_directory}/output/binaries"
	mkdir -p "${output_path}"
	python3 "${scripts_directory}/runDockerBuildInnerBuild.py" \
		--compiler-configuration "${compiler_configuration}" \
		--build-configuration "${build_configuration}" \
		--source-path "${git_root_directory}/client/catapult" \
		--out-dir "${output_path}"
fi
