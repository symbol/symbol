#!/bin/bash
# Java catbuffer generator runner (models). Mirrors sdk/javascript/scripts/run_catbuffer_generator.sh.
#
# Generates Java model classes for each blockchain (nem, symbol) by invoking catparser with the
# `generator.Generator` class from `sdk/java/generator/`. Output goes to
# `sdk/java/src/main/java/org/symbol/sdk/<blockchain>/models/` (one file per type, in a dedicated
# subpackage that keeps the hand-written runtime classes such as Address, KeyPair, Network, ...
# in the parent `org.symbol.sdk.<blockchain>` package uncluttered).
#
# The shared runner logic lives in run_catbuffer_common.sh; this wrapper only supplies the models config.

CATBUFFER_SCHEMA="all_generated.cats"
CATBUFFER_ARTIFACT="models"
CATBUFFER_DRYRUN_SUFFIX="_dryrun"
CATBUFFER_GENERATOR="generator.Generator"
CATBUFFER_SWEEP_GENERATOR="generator.ModelsSweepTestGenerator"
CATBUFFER_SWEEP_FILE="ModelsSweepTest.java"

# shellcheck source=scripts/run_catbuffer_common.sh
source "$(dirname "${BASH_SOURCE[0]}")/run_catbuffer_common.sh"
run_catbuffer "$@"
