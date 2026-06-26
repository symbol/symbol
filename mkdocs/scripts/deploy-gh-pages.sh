#!/bin/bash

set -ex

rm -rf ../docs ../docs-staging
SYMBOL_DOCS_DISABLE_TS=false mkdocs build -f config/mkdocs.en.yml
SYMBOL_DOCS_DISABLE_TS=false mkdocs build -f config/mkdocs.ja.yml
cd ..
mv docs docs-staging
git checkout gh-pages
rm -rf docs
mv docs-staging docs
git add -f docs
git commit -m "[docs] Update"
git push
git checkout new-docs
cd mkdocs
