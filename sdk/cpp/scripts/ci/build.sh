#!/bin/bash

set -ex

bash scripts/run_catbuffer_generator.sh dryrun
bash scripts/run_testvectors_generator.sh dryrun
bash scripts/run_examples_docs_generator.sh dryrun
