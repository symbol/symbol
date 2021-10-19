# Enumeration of Transaction types
enum TransactionType : uint16
	# AccountKeyLinkTransaction
	ACCOUNT_KEY_LINK = 0x414C

	# NodeKeyLinkTransaction
	NODE_KEY_LINK = 0x424C

	# AggregateCompleteTransaction
	AGGREGATE_COMPLETE = 0x4141

	# AggregateBondedTransaction
	AGGREGATE_BONDED = 0x4241

	# VotingKeyLinkTransaction
	VOTING_KEY_LINK = 0x4143

	# VrfKeyLinkTransaction
	VRF_KEY_LINK = 0x4243

	# HashLockTransaction
	HASH_LOCK = 0x4148

	# SecretLockTransaction
	SECRET_LOCK = 0x4152

	# SecretProofTransaction
	SECRET_PROOF = 0x4252

	# AccountMetadataTransaction
	ACCOUNT_METADATA = 0x4144

	# MosaicMetadataTransaction
	MOSAIC_METADATA = 0x4244

	# NamespaceMetadataTransaction
	NAMESPACE_METADATA = 0x4344

	# MosaicDefinitionTransaction
	MOSAIC_DEFINITION = 0x414D

	# MosaicSupplyChangeTransaction
	MOSAIC_SUPPLY_CHANGE = 0x424D

	# MosaicSupplyRevocationTransaction
	MOSAIC_SUPPLY_REVOCATION = 0x434D

	# MultisigAccountModificationTransaction
	MULTISIG_ACCOUNT_MODIFICATION = 0x4155

	# AddressAliasTransaction
	ADDRESS_ALIAS = 0x424E

	# MosaicAliasTransaction
	MOSAIC_ALIAS = 0x434E

	# NamespaceRegistrationTransaction
	NAMESPACE_REGISTRATION = 0x414E

	# AccountAddressRestrictionTransaction
	ACCOUNT_ADDRESS_RESTRICTION = 0x4150

	# AccountMosaicRestrictionTransaction
	ACCOUNT_MOSAIC_RESTRICTION = 0x4250

	# AccountOperationRestrictionTransaction
	ACCOUNT_OPERATION_RESTRICTION = 0x4350

	# MosaicAddressRestrictionTransaction
	MOSAIC_ADDRESS_RESTRICTION = 0x4251

	# MosaicGlobalRestrictionTransaction
	MOSAIC_GLOBAL_RESTRICTION = 0x4151

	# TransferTransaction
	TRANSFER = 0x4154
