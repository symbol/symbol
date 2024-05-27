#!/bin/bash

set -ex

PYTHONPATH=. python3 -m examples.bip32_keypair
PYTHONPATH=. python3 -m examples.transaction_aggregate --private examples/resources/zero.sha256.txt
PYTHONPATH=. python3 -m examples.transaction_multisig
PYTHONPATH=. python3 -m examples.transaction_sign --blockchain=nem
PYTHONPATH=. python3 -m examples.transaction_sign --blockchain=symbol

PYTHONPATH=. python3 -m examples.readme.nem
PYTHONPATH=. python3 -m examples.readme.symbol

PYTHONPATH=. python3 -m examples.docs
