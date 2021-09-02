import "types.cats"

# cosignature attached to an aggregate transaction
struct Cosignature
	# version
	version = uint64

	# cosigner public key
	signer_public_key = PublicKey

	# cosigner signature
	signature = Signature

# cosignature detached from an aggregate transaction
struct DetachedCosignature
	inline Cosignature

	# hash of the aggregate transaction that is signed by this cosignature
	parent_hash = Hash256
