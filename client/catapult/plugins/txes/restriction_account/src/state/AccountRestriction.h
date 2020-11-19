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
#include "AccountRestrictionDescriptor.h"
#include "src/model/AccountRestrictionModification.h"
#include <set>

namespace catapult { namespace state {

	/// Account restriction.
	class AccountRestriction {
	public:
		/// Raw restriction value.
		using RawValue = std::vector<uint8_t>;

	public:
		/// Creates an account restriction around \a restrictionFlags and \a restrictionValueSize.
		AccountRestriction(model::AccountRestrictionFlags restrictionFlags, size_t restrictionValueSize);

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
		bool canAllow(const model::AccountRestrictionModification& modification) const;

		/// Returns \c true if \a modification can be applied to the blocked values.
		bool canBlock(const model::AccountRestrictionModification& modification) const;

	public:
		/// Applies \a modification to the allowed values.
		void allow(const model::AccountRestrictionModification& modification);

		/// Applies \a modification to the the blocked values.
		void block(const model::AccountRestrictionModification& modification);

	private:
		bool isOperationAllowed(
				const model::AccountRestrictionModification& modification,
				AccountRestrictionOperationType operationType) const;

		void update(const model::AccountRestrictionModification& modification);

	private:
		AccountRestrictionDescriptor m_restrictionDescriptor;
		size_t m_restrictionValueSize;
		std::set<RawValue> m_values;
	};
}}
