#!/bin/bash

set -ex

BLOCKCHAIN=nem npm run coverage:vectors
BLOCKCHAIN=symbol npm run coverage:vectors
