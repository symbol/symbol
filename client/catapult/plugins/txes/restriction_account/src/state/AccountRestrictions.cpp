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

#include "AccountRestrictions.h"
#include "catapult/model/EntityType.h"

namespace catapult { namespace state {

	AccountRestrictions::AccountRestrictions(const Address& address)
			: m_address(address)
			, m_version(1) {
		constexpr auto Outgoing = model::AccountRestrictionFlags::Outgoing;

		addRestriction(model::AccountRestrictionFlags::Address, Address::Size);
		addRestriction(model::AccountRestrictionFlags::MosaicId, sizeof(MosaicId));
		addRestriction(model::AccountRestrictionFlags::Address | Outgoing, Address::Size);
		addRestriction(model::AccountRestrictionFlags::TransactionType | Outgoing, sizeof(model::EntityType));
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

	uint16_t AccountRestrictions::version() const {
		return m_version;
	}

	void AccountRestrictions::setVersion(uint16_t version) {
		m_version = version;
	}

	const AccountRestriction& AccountRestrictions::restriction(model::AccountRestrictionFlags restrictionFlags) const {
		return restriction<const RestrictionsMap, const AccountRestriction>(m_restrictions, restrictionFlags);
	}

	AccountRestriction& AccountRestrictions::restriction(model::AccountRestrictionFlags restrictionFlags) {
		return restriction<RestrictionsMap, AccountRestriction>(m_restrictions, restrictionFlags);
	}

	AccountRestrictions::const_iterator AccountRestrictions::begin() const {
		return m_restrictions.cbegin();
	}

	AccountRestrictions::const_iterator AccountRestrictions::end() const {
		return m_restrictions.cend();
	}

	void AccountRestrictions::addRestriction(model::AccountRestrictionFlags restrictionFlags, size_t restrictionValueSize) {
		m_restrictions.emplace(restrictionFlags, AccountRestriction(restrictionFlags, restrictionValueSize));
	}
}}
