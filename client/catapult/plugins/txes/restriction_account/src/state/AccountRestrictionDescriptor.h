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
#include "src/model/AccountRestrictionTypes.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	/// Operation type.
	enum class AccountRestrictionOperationType {
		/// Account restriction contains allowed values.
		Allow,

		/// Account restriction contains blocked value.
		Block
	};

	/// Account restriction descriptor.
	class AccountRestrictionDescriptor {
	public:
		/// Creates an account restriction descriptor around \a restrictionType.
		constexpr explicit AccountRestrictionDescriptor(model::AccountRestrictionType restrictionType)
				: m_restrictionType(restrictionType)
		{}

	public:
		/// Gets the value specific part of the restriction type.
		constexpr model::AccountRestrictionType restrictionType() const {
			return StripFlag(m_restrictionType, model::AccountRestrictionType::Block);
		}

		/// Gets the operation type.
		constexpr AccountRestrictionOperationType operationType() const {
			return model::HasFlag(model::AccountRestrictionType::Block, m_restrictionType)
					? AccountRestrictionOperationType::Block
					: AccountRestrictionOperationType::Allow;
		}

		/// Gets the raw restriction type.
		constexpr model::AccountRestrictionType raw() const {
			return m_restrictionType;
		}

	private:
		static constexpr model::AccountRestrictionType StripFlag(model::AccountRestrictionType lhs, model::AccountRestrictionType flag) {
			return static_cast<model::AccountRestrictionType>(utils::to_underlying_type(lhs) & ~utils::to_underlying_type(flag));
		}

	private:
		model::AccountRestrictionType m_restrictionType;
	};
}}
