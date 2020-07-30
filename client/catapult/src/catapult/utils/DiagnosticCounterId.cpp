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

#include "DiagnosticCounterId.h"
#include "catapult/exceptions.h"

namespace catapult { namespace utils {

	namespace {
		constexpr auto Counter_Name_Alphabet_Size = 27u; // ' ' [0] and A-Z [1-26]
	}

	DiagnosticCounterId::DiagnosticCounterId() : DiagnosticCounterId(0)
	{}

	DiagnosticCounterId::DiagnosticCounterId(const std::string& name) : m_name(name) {
		if (Max_Counter_Name_Size < m_name.size())
			CATAPULT_THROW_INVALID_ARGUMENT_1("counter name is too long", m_name);

		if (!m_name.empty() && (' ' == m_name.front() || ' ' == m_name.back()))
			CATAPULT_THROW_INVALID_ARGUMENT_1("counter cannot contain leading or trailing whitespace", m_name);

		m_value = 0;
		for (auto ch : m_name) {
			m_value *= Counter_Name_Alphabet_Size;
			if ('A' <= ch && ch <= 'Z')
				m_value += static_cast<uint64_t>(ch - 'A') + 1;
			else if (' ' != ch)
				CATAPULT_THROW_INVALID_ARGUMENT_1("counter name contains invalid char", ch);
		}
	}

	DiagnosticCounterId::DiagnosticCounterId(uint64_t value) : m_value(value) {
		for (auto i = 0u; i < Max_Counter_Name_Size; ++i) {
			if (0 == value)
				break;

			auto byte = value % Counter_Name_Alphabet_Size;
			m_name.insert(m_name.begin(), 0 == byte ? ' ' : static_cast<char>(byte - 1 + 'A'));
			value /= Counter_Name_Alphabet_Size;
		}

		if (0 != value)
			CATAPULT_THROW_INVALID_ARGUMENT_1("counter value is too large", m_value);
	}
}}
