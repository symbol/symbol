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
#include "AccountRestrictionDescriptor.h"
#include <set>
#include <vector>

namespace catapult { namespace state {

	/// Account restriction.
	class AccountRestriction {
	public:
		/// Raw restriction value.
		using RawValue = std::vector<uint8_t>;

	public:
		/// Creates an account restriction around \a restrictionType and \a restrictionValueSize.
		AccountRestriction(model::AccountRestrictionType restrictionType, size_t restrictionValueSize);

	public:
		/// Gets the restriction descriptor.
		const AccountRestrictionDescriptor& descriptor() const;

		/// Gets the restriction value size.
		size_t valueSize() const;

		/// Gets the values.
		const std::set<std::vector<uint8_t>>& values() const;

	public:
		/// Returns \c true if \a value is known.
		bool contains(const std::vector<uint8_t>& value) const;

		/// Returns \c true if \a modification can be applied to the allowed values.
		bool canAllow(const model::RawAccountRestrictionModification& modification) const;

		/// Returns \c true if \a modification can be applied to the blocked values.
		bool canBlock(const model::RawAccountRestrictionModification& modification) const;

	public:
		/// Applies \a modification to the allowed values.
		void allow(const model::RawAccountRestrictionModification& modification);

		/// Applies \a modification to the the blocked values.
		void block(const model::RawAccountRestrictionModification& modification);

	private:
		bool isOperationAllowed(
				const model::RawAccountRestrictionModification& modification,
				AccountRestrictionOperationType operationType) const;

		void update(const model::RawAccountRestrictionModification& modification);

	private:
		AccountRestrictionDescriptor m_restrictionDescriptor;
		size_t m_restrictionValueSize;
		std::set<RawValue> m_values;
	};
}}
