#pragma once
#include "VerifiableEntity.h"
#include "catapult/preprocessor.h"
#include <functional>

namespace catapult { namespace model {

	/// Prototype for a verifiable entity predicate.
	using VerifiableEntityPredicate = std::function<bool (const VerifiableEntity&)>;

	/// Creates a predicate that always returns \c true.
	CATAPULT_INLINE
	VerifiableEntityPredicate NeverFilter() {
		return [](const auto&) { return true; };
	}

	/// Creates a predicate that returns \c true when an entity has a matching entity \a type.
	CATAPULT_INLINE
	VerifiableEntityPredicate HasTypeFilter(EntityType type) {
		return [type](const auto& entity) { return entity.Type == type; };
	}

	/// Creates a predicate that returns \c true when an entity has a matching basic entity \a type.
	CATAPULT_INLINE
	VerifiableEntityPredicate HasBasicTypeFilter(BasicEntityType type) {
		return [type](const auto& entity) { return ToBasicEntityType(entity.Type) == type; };
	}
}}
