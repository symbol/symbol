#!/bin/bash

set -ex

PYTHONPATH=. python3 examples/bip32_keypair.py
PYTHONPATH=. python3 examples/transaction_aggregate.py --private examples/resources/zero.sha256.txt
PYTHONPATH=. python3 examples/transaction_multisig.py
PYTHONPATH=. python3 examples/transaction_sign.py --blockchain=nem
PYTHONPATH=. python3 examples/transaction_sign.py --blockchain=symbol
