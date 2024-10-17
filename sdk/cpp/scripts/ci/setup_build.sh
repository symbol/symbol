#!/bin/bash

set -ex

python3 -m pip install -r "$(git rev-parse --show-toplevel)/catbuffer/parser/requirements.txt"
