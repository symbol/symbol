#!/bin/bash

set -ex

rm -rf catapult-src/internal

branchName=$(echo ${GIT_BRANCH} | sed 's/origin\///;')
hasInternal=$(GIT_SSH_COMMAND="ssh -i ${CREDS_KEYFILE}" git ls-remote --heads ${INTERNAL_REPO} refs/heads/${branchName} | wc -l)
if [ "${hasInternal}" == "1" ];
then
	GIT_SSH_COMMAND="ssh -i ${CREDS_KEYFILE}" git clone -b ${branchName} ${INTERNAL_REPO} catapult-src/internal
else
	GIT_SSH_COMMAND="ssh -i ${CREDS_KEYFILE}" git clone ${INTERNAL_REPO} catapult-src/internal
fi

scriptName="${1}"
shift
ls catapult-src/internal
exec bash ./catapult-src/internal/scripts/build/${scriptName} $*
