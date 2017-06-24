#pragma once
#include "ValidationResult.h"
#include "catapult/model/WeakEntityInfo.h"
#include <string>

namespace catapult { namespace validators {

	/// A weakly typed entity validator.
	/// \note This is intended to be used only for stateless validation.
	template<typename... TArgs>
	class EntityValidatorT {
	public:
		virtual ~EntityValidatorT() {}

	public:
		/// Gets the validator name.
		virtual const std::string& name() const = 0;

		/// Validates a single \a entityInfo with contextual information \a args.
		virtual ValidationResult validate(const model::WeakEntityInfo& entityInfo, TArgs&&... args) const = 0;
	};
}}
