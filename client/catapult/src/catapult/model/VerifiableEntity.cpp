#include "VerifiableEntity.h"
#include "Block.h"
#include "Transaction.h"

namespace catapult { namespace model {

	std::ostream& operator<<(std::ostream& out, const VerifiableEntity& entity) {
		out << entity.Type << " (v" << static_cast<uint16_t>(entity.EntityVersion()) << ") with size " << entity.Size;
		return out;
	}

	bool IsSizeValid(const VerifiableEntity& entity, const TransactionRegistry& registry) {
		switch (ToBasicEntityType(entity.Type)) {
		case BasicEntityType::Block:
			return IsSizeValid(static_cast<const Block&>(entity), registry);
		case BasicEntityType::Transaction:
			return IsSizeValid(static_cast<const Transaction&>(entity), registry);
		default:
			return false;
		}
	}
}}
