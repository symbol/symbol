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
#include "TypedAccountRestriction.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Account restrictions.
	class AccountRestrictions {
	private:
		using RestrictionsMap = std::map<model::AccountRestrictionType, AccountRestriction>;
		using const_iterator = RestrictionsMap::const_iterator;

	public:
		/// Creates account restrictions around \a address.
		explicit AccountRestrictions(const Address& address);

	public:
		/// Gets the account address.
		const Address& address() const;

		/// Gets the number of possible account restrictions.
		size_t size() const;

		/// Returns \c true if no restrictions are set.
		bool isEmpty() const;

	public:
		/// Gets the const typed account restriction specified by \a restrictionType.
		template<typename TRestrictionValue>
		TypedAccountRestriction<TRestrictionValue> restriction(model::AccountRestrictionType restrictionType) const {
			const auto& restriction = this->restriction<const RestrictionsMap, const AccountRestriction>(m_restrictions, restrictionType);
			return TypedAccountRestriction<TRestrictionValue>(restriction);
		}

		/// Gets the const account restriction specified by \a restrictionType.
		const AccountRestriction& restriction(model::AccountRestrictionType restrictionType) const;

		/// Gets the account restriction specified by \a restrictionType.
		AccountRestriction& restriction(model::AccountRestrictionType restrictionType);

	public:
		/// Gets a const iterator to the first account restriction.
		const_iterator begin() const;

		/// Gets a const iterator to the element following the last account restriction.
		const_iterator end() const;

	private:
		void addRestriction(model::AccountRestrictionType restrictionType, size_t restrictionValueSize);

	private:
		template<typename TRestrictionsMap, typename TAccountRestriction>
		static TAccountRestriction& restriction(TRestrictionsMap& restrictionsMap, model::AccountRestrictionType restrictionType) {
			auto iter = restrictionsMap.find(AccountRestrictionDescriptor(restrictionType).directionalRestrictionType());
			if (restrictionsMap.cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown account restriction type", static_cast<uint16_t>(restrictionType));

			return iter->second;
		}

	private:
		Address m_address;
		RestrictionsMap m_restrictions;
	};
}}
