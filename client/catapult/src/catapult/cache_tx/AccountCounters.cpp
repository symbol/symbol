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

#include "AccountCounters.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	AccountCounters::AccountCounters() : m_totalUseCount(0)
	{}

	size_t AccountCounters::size() const {
		return m_accountCounters.size();
	}

	size_t AccountCounters::deepSize() const {
		return m_totalUseCount;
	}

	size_t AccountCounters::count(const Key& key) const {
		auto iter = m_accountCounters.find(key);
		return m_accountCounters.cend() == iter ? 0 : iter->second;
	}

	void AccountCounters::increment(const Key& key) {
		++m_accountCounters[key];
		++m_totalUseCount;
	}

	void AccountCounters::decrement(const Key& key) {
		auto& useCount = m_accountCounters[key];
		if (0 == useCount)
			CATAPULT_THROW_RUNTIME_ERROR_1("use count cannot be decremented below zero for key", key);

		--useCount;
		--m_totalUseCount;

		if (0 == useCount)
			m_accountCounters.erase(key);
	}

	void AccountCounters::reset() {
		m_accountCounters.clear();
		m_totalUseCount = 0;
	}
}}
