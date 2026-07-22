#!/bin/bash

set -ex

bash scripts/run_catbuffer_generator.sh dryrun
bash scripts/run_catbuffer_descriptor_generator.sh dryrun

./gradlew --no-daemon assemble
