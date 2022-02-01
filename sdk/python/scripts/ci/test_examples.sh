#!/bin/bash

set -ex

PYTHONPATH=. coverage run --append examples/bip32_keypair.py
PYTHONPATH=. coverage run --append examples/transaction_aggregate.py --private examples/resources/zero.sha256.txt
PYTHONPATH=. coverage run --append examples/transaction_multisig.py
PYTHONPATH=. coverage run --append examples/transaction_sign.py --blockchain=nem
PYTHONPATH=. coverage run --append examples/transaction_sign.py --blockchain=symbol
