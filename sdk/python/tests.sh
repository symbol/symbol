#!/bin/bash

PYTHONPATH=. SCHEMAS_PATH="../../catbuffer/schemas/test-vectors" pytest tests/generator

python3 -m unittest discover -v

