#!/bin/bash

set -ex

# Run from the mkdocs folder
export NO_MKDOCS_2_WARNING=1
mkdocs build -f config/mkdocs.en.yml
mkdocs build -f config/mkdocs.ja.yml
