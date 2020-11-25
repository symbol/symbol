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
#include "AccountRestriction.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Account restrictions.
	class PLUGIN_API_DEPENDENCY AccountRestrictions {
	private:
		using RestrictionsMap = std::map<model::AccountRestrictionFlags, AccountRestriction>;
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
		/// Gets the serialization version.
		uint16_t version() const;

		/// Sets the serialization \a version.
		void setVersion(uint16_t version);

		/// Gets the const account restriction specified by \a restrictionFlags.
		const AccountRestriction& restriction(model::AccountRestrictionFlags restrictionFlags) const;

		/// Gets the account restriction specified by \a restrictionFlags.
		AccountRestriction& restriction(model::AccountRestrictionFlags restrictionFlags);

	public:
		/// Gets a const iterator to the first account restriction.
		const_iterator begin() const;

		/// Gets a const iterator to the element following the last account restriction.
		const_iterator end() const;

	private:
		void addRestriction(model::AccountRestrictionFlags restrictionFlags, size_t restrictionValueSize);

	private:
		template<typename TRestrictionsMap, typename TAccountRestriction>
		static TAccountRestriction& restriction(TRestrictionsMap& restrictionsMap, model::AccountRestrictionFlags restrictionFlags) {
			auto iter = restrictionsMap.find(AccountRestrictionDescriptor(restrictionFlags).directionalRestrictionFlags());
			if (restrictionsMap.cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown account restriction flags", utils::to_underlying_type(restrictionFlags));

			return iter->second;
		}

	private:
		Address m_address;
		uint16_t m_version;
		RestrictionsMap m_restrictions;
	};
}}
