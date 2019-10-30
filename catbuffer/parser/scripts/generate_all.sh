#!/bin/bash

source "$(dirname $0)/schema_lists.sh"
source "$(dirname $0)/generate_batch.sh"

generate_batch block_inputs "."
generate_batch receipt_inputs "."
generate_batch transaction_inputs "."
