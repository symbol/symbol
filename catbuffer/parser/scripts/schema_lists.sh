#!/bin/bash

block_inputs=(
	"block")

receipt_inputs=(
	"receipts"
	"namespace/namespace_receipts")

transaction_inputs=(
	"account_link/account_link"
	"aggregate/aggregate"
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


state_inputs=(
	"state/resriction_account_serializer"
	"state/account_state_serializer"
	"state/hash_lock_serializer"
	"state/lock_info_serializer"
	"state/metadata_entry_serializer"
	"state/mosaic_entry_serializer"
	"state/mosaic_restriction_entry_serializer"
	"state/multisig_entry_serializer"
	"state/namespace_history_serializer"
	"state/secret_lock_serializer"
)
