#!/bin/bash

set -ex

rm -rf catapult-src/internal

branchName="$(echo "${GIT_BRANCH}" | sed 's/origin\///;')"

# pick main as default
internalBranchName="main"

# if there's corresponding internal branch to main repo branch pick it instead
hasInternal=$(GIT_SSH_COMMAND="ssh -i ${CREDS_KEYFILE}" git ls-remote --heads "${INTERNAL_REPO}" "refs/heads/${branchName}" | wc -l)
if [ "${hasInternal}" == "1" ]; then
    internalBranchName="${branchName}"
fi

GIT_SSH_COMMAND="ssh -i ${CREDS_KEYFILE}" git clone -b "${internalBranchName}" "${INTERNAL_REPO}" catapult-src/internal

pushd catapult-src/internal

git log --oneline -1

popd

scriptName="${1}"
shift
ls catapult-src/internal
exec bash "./catapult-src/internal/scripts/build/${scriptName}" "$@"
