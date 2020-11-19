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
#include "src/model/AccountRestrictionFlags.h"
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
		/// Creates an account restriction descriptor around \a restrictionFlags.
		constexpr explicit AccountRestrictionDescriptor(model::AccountRestrictionFlags restrictionFlags)
				: m_restrictionFlags(restrictionFlags)
		{}

	public:
		/// Gets the value specific part of the restriction flags including the direction.
		constexpr model::AccountRestrictionFlags directionalRestrictionFlags() const {
			return StripFlag(m_restrictionFlags, model::AccountRestrictionFlags::Block);
		}

		/// Gets the value specific part of the restriction flags excluding the direction.
		constexpr model::AccountRestrictionFlags restrictionFlags() const {
			return StripFlag(m_restrictionFlags, model::AccountRestrictionFlags::Outgoing | model::AccountRestrictionFlags::Block);
		}

		/// Gets the operation type.
		constexpr AccountRestrictionOperationType operationType() const {
			return model::HasFlag(model::AccountRestrictionFlags::Block, m_restrictionFlags)
					? AccountRestrictionOperationType::Block
					: AccountRestrictionOperationType::Allow;
		}

		/// Gets the raw restriction flags.
		constexpr model::AccountRestrictionFlags raw() const {
			return m_restrictionFlags;
		}

	private:
		static constexpr model::AccountRestrictionFlags StripFlag(
				model::AccountRestrictionFlags lhs,
				model::AccountRestrictionFlags flag) {
			return static_cast<model::AccountRestrictionFlags>(utils::to_underlying_type(lhs) & ~utils::to_underlying_type(flag));
		}

	private:
		model::AccountRestrictionFlags m_restrictionFlags;
	};
}}
