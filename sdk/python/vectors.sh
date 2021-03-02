#!/bin/bash

python3 -m vectors.all --blockchain nis1 --vectors ../test-vectors/nis1 --tests 0 1 2 3 4 5
python3 -m vectors.all --blockchain symbol --vectors ../test-vectors
