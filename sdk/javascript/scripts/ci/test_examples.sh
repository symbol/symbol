#!/bin/bash

set -ex

node examples/bip32_keypair.js
node examples/transaction_aggregate.js --private examples/resources/zero.sha256.txt
node examples/transaction_multisig.js
node examples/transaction_sign.js --blockchain=nem
node examples/transaction_sign.js --blockchain=symbol
