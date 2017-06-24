#include "EntityTestUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace test {

	Timestamp GetTag(const model::VerifiableEntity& entity) {
		switch (model::ToBasicEntityType(entity.Type)) {
		case model::BasicEntityType::Block:
			return static_cast<const model::Block&>(entity).Timestamp;

		case model::BasicEntityType::Transaction:
			return static_cast<const model::Transaction&>(entity).Deadline;

		default:
			CATAPULT_THROW_RUNTIME_ERROR("GetTag was called with unexpected entity type");
		}
	}
}}
