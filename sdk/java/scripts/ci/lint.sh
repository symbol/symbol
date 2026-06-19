#!/bin/bash

set -ex

./gradlew --no-daemon spotlessCheck
