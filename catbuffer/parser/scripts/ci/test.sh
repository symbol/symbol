#!/bin/bash

set -ex

coverage run -m unittest discover -v
