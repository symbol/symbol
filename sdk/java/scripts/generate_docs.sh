#!/bin/bash

set -ex

# ':javadoc' scopes to the root project; the examples subproject is not part of the published API docs
./gradlew --no-daemon :javadoc
