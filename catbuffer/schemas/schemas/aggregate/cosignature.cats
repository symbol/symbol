import "types.cats"

# a cosignature
struct Cosignature
	# cosigner public key
	signer = Key

	# cosigner signature
	signature = Signature

# a detached cosignature
struct DetachedCosignature
	inline Cosignature

	# hash of the corresponding parent
	parentHash = Hash256
