/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "ValidatorTypes.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/utils/NamedObject.h"
#include <memory>

namespace catapult { namespace validators {

	/// An aggregate validator.
	template<typename... TArgs>
	class AggregateEntityValidatorT final {
	public:
		using ValidatorVector = ValidatorVectorT<TArgs...>;

	public:
		/// Creates an aggregate validator around \a validators.
		explicit AggregateEntityValidatorT(ValidatorVector&& validators)
				: m_validators(std::move(validators))
		{}

	public:
		/// Helper for invoking curried validators.
		class DispatchForwarder {
		public:
			/// Creates a forwarder around \a validators.
			explicit DispatchForwarder(ValidationFunctions&& validationFunctions)
					: m_validationFunctions(std::move(validationFunctions))
			{}

		public:
			/// Dispatches validation of \a entityInfos to \a dispatcher.
			template<typename TDispatcher>
			auto dispatch(const TDispatcher& dispatcher, const model::WeakEntityInfos& entityInfos) const {
				return dispatcher(entityInfos, m_validationFunctions);
			}

		private:
			ValidationFunctions m_validationFunctions;
		};

#ifdef __clang__
#pragma clang diagnostic push
// clang generates this warning when curry() is called without arguments
#pragma clang diagnostic ignored "-Wunused-lambda-capture"
#endif

		/// Prepares the invocation of sub validators by currying \a args to invocations made on the returned forwarder.
		template<typename... TCurryArgs>
		DispatchForwarder curry(TCurryArgs&&... args) const {
			ValidationFunctions validationFunctions;
			validationFunctions.reserve(m_validators.size());
			auto forwardedArgs = std::make_tuple(std::forward<TCurryArgs>(args)...);
			for (const auto& pValidator : m_validators) {
				validationFunctions.emplace_back([&pValidator, forwardedArgs](const auto& entityInfo) {
					return pValidator->validate(entityInfo, std::get<TArgs>(forwardedArgs)...);
				});
			}

			return DispatchForwarder(std::move(validationFunctions));
		}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

		/// Gets the names of all sub validators.
		std::vector<std::string> names() const {
			return utils::ExtractNames(m_validators);
		}

	private:
		ValidatorVector m_validators;
	};
}}
