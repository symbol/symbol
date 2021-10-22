import "types.cats"

# Cosignature attached to an AggregateCompleteTransaction or AggregateBondedTransaction.
struct Cosignature
	# Version.
	version = uint64

	# Cosigner public key.
	signer_public_key = PublicKey

	# Transaction signature.
	signature = Signature

# Cosignature detached from an AggregateCompleteTransaction or AggregateBondedTransaction.
struct DetachedCosignature
	inline Cosignature

	# Hash of the AggregateBondedTransaction that is signed by this cosignature.
	parent_hash = Hash256
