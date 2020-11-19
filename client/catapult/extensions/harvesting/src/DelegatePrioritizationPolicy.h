/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/types.h"
#include <functional>
#include <iosfwd>

namespace catapult { namespace cache { class CatapultCache; } }

namespace catapult { namespace harvesting {

	// region DelegatePrioritizationPolicy

#define DELEGATE_PRIORITIZATION_POLICY_LIST \
	/* Prioritize older delegates over newer delegates. */ \
	ENUM_VALUE(Age) \
	\
	/* Prioritize more important delegates over less important delegates. */ \
	ENUM_VALUE(Importance)

#define ENUM_VALUE(LABEL) LABEL,
	/// Possible delegate prioritization policies.
	enum class DelegatePrioritizationPolicy : uint8_t {
		DELEGATE_PRIORITIZATION_POLICY_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, DelegatePrioritizationPolicy value);

	/// Tries to parse \a str into connection security \a policy.
	bool TryParseValue(const std::string& str, DelegatePrioritizationPolicy& policy);

	// endregion

	// region CreateDelegatePrioritizer

	/// Looks up a prioritization score for an account.
	using DelegatePrioritizer = std::function<uint64_t (const Key&)>;

	/// Creates a delegate prioritizer for specified \a policy given \a cache and \a primaryAccountPublicKey.
	DelegatePrioritizer CreateDelegatePrioritizer(
			DelegatePrioritizationPolicy policy,
			const cache::CatapultCache& cache,
			const Key& primaryAccountPublicKey);

	// endregion
}}
