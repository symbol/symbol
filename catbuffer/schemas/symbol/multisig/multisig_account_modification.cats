import "transaction.cats"

# Shared content between MultisigAccountModificationTransaction and EmbeddedMultisigAccountModificationTransaction.
inline struct MultisigAccountModificationTransactionBody
	# Relative change to the **minimum** number of cosignatures required when **removing a cosignatory**.
	#
	# E.g., when moving from 0 to 2 cosignatures this number would be **2**.
	# When moving from 4 to 3 cosignatures, the number would be **-1**.
	min_removal_delta = int8

	# Relative change to the **minimum** number of cosignatures required when **approving a transaction**.
	#
	# E.g., when moving from 0 to 2 cosignatures this number would be **2**.
	# When moving from 4 to 3 cosignatures, the number would be **-1**.
	min_approval_delta = int8

	# Number of cosignatory address additions.
	address_additions_count = uint8

	# Number of cosignatory address deletions.
	address_deletions_count = uint8

	# Reserved padding to align addressAdditions to an 8-byte boundary.
	multisig_account_modification_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# Cosignatory address additions.
	#
	# All accounts in this list will be able to cosign transactions on behalf of the multisig account.
	# The number of required cosignatures depends on the configured minimum approval and minimum removal values.
	address_additions = array(UnresolvedAddress, address_additions_count)

	# Cosignatory address deletions.
	#
	# All accounts in this list will stop being able to cosign transactions on behalf of the multisig account.
	# A transaction containing **any** address in this array requires a number of cosignatures at least equal
	# to the minimum removal value.
	address_deletions = array(UnresolvedAddress, address_deletions_count)

# Create or modify a [multi-signature](/concepts/multisig-account.html) account.
#
# This transaction allows you to:
# - Transform a regular account into a multisig account.
# - Change the configurable properties of a multisig account.
# - Add or delete cosignatories from a multisig account
#   (removing all cosignatories turns a multisig account into a regular account again).
struct MultisigAccountModificationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_ACCOUNT_MODIFICATION)

	inline Transaction
	inline MultisigAccountModificationTransactionBody

# Embedded version of MultisigAccountModificationTransaction.
struct EmbeddedMultisigAccountModificationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_ACCOUNT_MODIFICATION)

	inline EmbeddedTransaction
	inline MultisigAccountModificationTransactionBody
