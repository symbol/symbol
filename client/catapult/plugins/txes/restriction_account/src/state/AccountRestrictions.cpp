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

#include "AccountRestrictions.h"
#include "catapult/model/EntityType.h"

namespace catapult { namespace state {

	AccountRestrictions::AccountRestrictions(const Address& address) : m_address(address) {
		addRestriction(model::AccountRestrictionType::Address, Address_Decoded_Size);
		addRestriction(model::AccountRestrictionType::MosaicId, sizeof(MosaicId));
		addRestriction(model::AccountRestrictionType::TransactionType, sizeof(model::EntityType));
	}

	const Address& AccountRestrictions::address() const {
		return m_address;
	}

	size_t AccountRestrictions::size() const {
		return m_restrictions.size();
	}

	bool AccountRestrictions::isEmpty() const {
		return std::all_of(begin(), end(), [](const auto& pair) {
			return pair.second.values().empty();
		});
	}

	const AccountRestriction& AccountRestrictions::restriction(model::AccountRestrictionType restrictionType) const {
		return restriction<const RestrictionsMap, const AccountRestriction>(m_restrictions, restrictionType);
	}

	AccountRestriction& AccountRestrictions::restriction(model::AccountRestrictionType restrictionType) {
		return restriction<RestrictionsMap, AccountRestriction>(m_restrictions, restrictionType);
	}

	AccountRestrictions::const_iterator AccountRestrictions::begin() const {
		return m_restrictions.cbegin();
	}

	AccountRestrictions::const_iterator AccountRestrictions::end() const {
		return m_restrictions.cend();
	}

	void AccountRestrictions::addRestriction(model::AccountRestrictionType restrictionType, size_t restrictionValueSize) {
		m_restrictions.emplace(restrictionType, AccountRestriction(restrictionType, restrictionValueSize));
	}
}}
