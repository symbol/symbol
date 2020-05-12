#!/bin/bash

block_inputs=(
	"block")

receipt_inputs=(
	"receipts"
	"namespace/namespace_receipts"
	"resolution_statement/resolution_statements")

state_inputs=(
	"state/account_state"
	"state/hash_lock"
	"state/lock_info"
	"state/metadata_entry"
	"state/mosaic_entry"
	"state/multisig_entry"
	"state/namespace_history"
	"state/restriction_account"
	"state/restriction_mosaic_entry"
	"state/secret_lock")

transaction_inputs=(
	"account_link/account_key_link"
	"account_link/node_key_link"
	"aggregate/aggregate"
	"coresystem/voting_key_link"
	"coresystem/vrf_key_link"
	"lock_hash/hash_lock"
	"lock_secret/secret_lock"
	"lock_secret/secret_proof"
	"metadata/account_metadata"
	"metadata/mosaic_metadata"
	"metadata/namespace_metadata"
	"mosaic/mosaic_definition"
	"mosaic/mosaic_supply_change"
	"multisig/multisig_account_modification"
	"namespace/address_alias"
	"namespace/mosaic_alias"
	"namespace/namespace_registration"
	"restriction_account/account_address_restriction"
	"restriction_account/account_mosaic_restriction"
	"restriction_account/account_operation_restriction"
	"restriction_mosaic/mosaic_address_restriction"
	"restriction_mosaic/mosaic_global_restriction"
	"transfer/transfer")
