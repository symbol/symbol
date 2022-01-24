#!/bin/bash

set -ex

gitlint -C "$(git rev-parse --show-toplevel)/linters/git/.gitlint"
