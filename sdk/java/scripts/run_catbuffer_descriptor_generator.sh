#!/bin/bash
# Java catbuffer typed-descriptor generator runner. Mirrors
# sdk/javascript/scripts/run_catbuffer_generator_ts.sh.
#
# Generates Java typed-descriptor classes for each blockchain (nem, symbol) by invoking catparser
# with the `generator.DescriptorGenerator` class from `sdk/java/generator/` (same Python package
# as the model generator). Output goes to `sdk/java/src/main/java/org/symbol/sdk/<blockchain>/
# descriptors/` (a dedicated subpackage kept separate from the generated `models/` subpackage
# and the hand-written runtime classes).
#
# The shared runner logic lives in run_catbuffer_common.sh; this wrapper only supplies the descriptor config.

CATBUFFER_SCHEMA="all_transactions.cats"
CATBUFFER_ARTIFACT="descriptors"
CATBUFFER_DRYRUN_SUFFIX="_descriptor_dryrun"
CATBUFFER_GENERATOR="generator.DescriptorGenerator"
CATBUFFER_SWEEP_GENERATOR="generator.DescriptorSweepTestGenerator"
CATBUFFER_SWEEP_FILE="DescriptorsSweepTest.java"

# shellcheck source=scripts/run_catbuffer_common.sh
source "$(dirname "${BASH_SOURCE[0]}")/run_catbuffer_common.sh"
run_catbuffer "$@"
