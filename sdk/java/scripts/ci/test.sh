#!/bin/bash

set -ex

# jacoco coverage report is always produced as `test` is finalized by `jacocoTestReport` in build.gradle.kts
./gradlew --no-daemon test
