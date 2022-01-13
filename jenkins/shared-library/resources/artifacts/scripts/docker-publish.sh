#!/usr/bin/env bash

set -x 

SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# shellcheck source=./shared-library/resources/artifacts/scripts/docker-functions.sh
source "$SCRIPTS_DIR/docker-functions.sh"
docker_build "$(load_version_from_file)" "$1"
