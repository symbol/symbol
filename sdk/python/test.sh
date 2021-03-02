#!/bin/bash

python3 -m unittest discover -v ./test/core
python3 -m unittest discover -v ./test/nis1
python3 -m unittest discover -v ./test/sym
