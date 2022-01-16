#!/bin/bash

set -ex

./_build/bin/catapult.tools.testvectors --vectors "$(git rev-parse --show-toplevel)/tests/vectors"
