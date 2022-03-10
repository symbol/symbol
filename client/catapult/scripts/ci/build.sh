#!/bin/bash

set -ex

git_root_directory="$(git rev-parse --show-toplevel)"
scripts_directory="${git_root_directory}/jenkins/catapult"
build_configuration="${scripts_directory}/configurations/tests-diagnostics.yaml"
if [ -z "${JENKINS_HOME}" ]
then
	python3 "${scripts_directory}/runDockerBuild.py" \
		--compiler-configuration "${scripts_directory}/configurations/gcc-10.yaml" \
		--build-configuration "${build_configuration}" \
		--operating-system ubuntu \
		--user "$(id -u):$(id -g)" \
		--destination-image-label gcc-10-local \
		--source-path "${git_root_directory}"
else
	output_path="${git_root_directory}/output/binaries"
	mkdir -p "${output_path}"
	python3 "${scripts_directory}/runDockerBuildInnerBuild.py" \
		--compiler-configuration "${scripts_directory}/configurations/gcc-11.yaml" \
		--build-configuration "${build_configuration}" \
		--source-path "${git_root_directory}/client/catapult" \
		--out-dir "${output_path}"
fi
