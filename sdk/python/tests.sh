#!/bin/bash

python3 -m unittest discover -v ./tests/core
python3 -m unittest discover -v ./tests/nis1
python3 -m unittest discover -v ./tests/sym
