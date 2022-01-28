#!/bin/bash

set -ex

#BLOCKCHAIN=nem npm run txvectors
BLOCKCHAIN=symbol npm run catvectors

BLOCKCHAIN=nem npm run vectors
BLOCKCHAIN=symbol npm run vectors
