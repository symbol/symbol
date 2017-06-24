#pragma once
#include "EmbeddedEntity.h"

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace model { class TransactionRegistry; }
}

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a verifiable entity header.
	struct VerifiableEntityHeader : public SizePrefixedEntity {
	public:
		/// The size of the VerifiableEntity part that can be skipped when signing/verifying.
		static constexpr size_t Header_Size = sizeof(uint32_t) + Signature_Size + Key_Size;

		/// The entity signature.
		catapult::Signature Signature;
	};

	/// Binary layout for a verifiable (with signature) entity.
	struct VerifiableEntity : public EntityBody<VerifiableEntityHeader> {
	};

#pragma pack(pop)

	/// Insertion operator for outputting \a entity to \a out.
	std::ostream& operator<<(std::ostream& out, const VerifiableEntity& entity);

	/// Checks the real size of \a entity against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const VerifiableEntity& entity, const TransactionRegistry& registry);
}}
