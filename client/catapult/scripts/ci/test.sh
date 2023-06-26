#!/bin/bash

set -ex

# $1 is test mode but we don't use it
architecture="$2"
git_root_directory="$(git rev-parse --show-toplevel)"
scripts_directory="${git_root_directory}/jenkins/catapult"
compiler_configuration="${scripts_directory}/configurations/${architecture}/gcc-latest.yaml"
if [ -z "${JENKINS_HOME}" ]
then
	python3 "${scripts_directory}/runDockerTests.py" \
		--image symbolplatform/symbol-server-test:gcc-latest-local \
		--compiler-configuration "${compiler_configuration}" \
		--user "$(id -u):$(id -g)" \
		--mode test \
		--verbosity suite \
		--source-path "${git_root_directory}"
else
	output_path="${git_root_directory}/output"
	data_path="${output_path}/catapult-data"
	mkdir -p "${data_path}/workdir"
	cd "${data_path}/workdir"
	python3 "${scripts_directory}/runDockerTestsInnerTest.py" \
		--compiler-configuration "${compiler_configuration}" \
		--exe-path "${output_path}/binaries/tests" \
		--out-dir "${data_path}" \
		--source-path "${git_root_directory}/client/catapult" \
		--verbosity suite
fi
