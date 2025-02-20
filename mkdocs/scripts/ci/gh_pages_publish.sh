#!/bin/bash

set -ex

git commit -m "[docs]: release new docs"
git remote set-url origin https://github.com/symbol/symbol
git remote -v
git push -f origin symbol:gh-pages
