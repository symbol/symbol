#!/bin/bash

set -ex

rm -rf catapult-src/internal
GIT_SSH_COMMAND="ssh -i ${CREDS_KEYFILE}" git clone ${INTERNAL_REPO} catapult-src/internal
scriptName="${1}"
shift
ls catapult-src/internal
exec bash ./catapult-src/internal/scripts/build/${scriptName} $*
