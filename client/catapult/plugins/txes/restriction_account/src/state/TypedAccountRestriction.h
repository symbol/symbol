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
#include "AccountRestriction.h"
#include "AccountRestrictionUtils.h"

namespace catapult { namespace state {

	/// Typed account restriction.
	template<typename TRestrictionValue>
	class TypedAccountRestriction {
	public:
		/// Creates a typed account restriction around \a restriction.
		explicit TypedAccountRestriction(const AccountRestriction& restriction) : m_restriction(restriction)
		{}

	public:
		/// Gets the descriptor of the underlying account restriction.
		const AccountRestrictionDescriptor& descriptor() const {
			return m_restriction.descriptor();
		}

		/// Gets the number of values of the underlying account restriction.
		size_t size() const {
			return m_restriction.values().size();
		}

		/// Returns \c true if the underlying account restriction contains \a value.
		bool contains(const TRestrictionValue& value) const {
			return m_restriction.contains(ToVector(value));
		}

	public:
		/// Returns \c true if \a modification can be applied to the underlying restriction.
		bool canAllow(const model::AccountRestrictionModification<TRestrictionValue>& modification) const {
			return m_restriction.canAllow({ modification.ModificationType, ToVector(modification.Value) });
		}

		/// Returns \c true if \a modification can be applied to the underlying restriction.
		bool canBlock(const model::AccountRestrictionModification<TRestrictionValue>& modification) const {
			return m_restriction.canBlock({ modification.ModificationType, ToVector(modification.Value) });
		}

	private:
		const AccountRestriction& m_restriction;
	};
}}
