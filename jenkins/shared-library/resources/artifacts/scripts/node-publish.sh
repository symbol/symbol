#!/usr/bin/env bash

set -x 

SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# shellcheck source=./shared-library/resources/artifacts/scripts/node-functions.sh
source "$SCRIPTS_DIR/node-functions.sh"
npm pack
bash ./node-functions.sh "$1"
