#!/bin/bash

set -ex

! rg --files-with-matches '^  ' "$(git rev-parse --show-toplevel)" | grep -vE ".json|.md|.LESSER|.eslintrc|.yaml|.txt"
