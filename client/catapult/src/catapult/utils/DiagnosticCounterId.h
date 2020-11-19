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
#include <string>

namespace catapult { namespace utils {

	/// Diagnostic counter id.
	class DiagnosticCounterId {
	public:
		/// Maximum counter name size.
		static constexpr auto Max_Counter_Name_Size = 13u;

	public:
		/// Creates an empty id.
		DiagnosticCounterId();

		/// Creates an id from \a name.
		explicit DiagnosticCounterId(const std::string& name);

		/// Creates an id from \a value.
		explicit DiagnosticCounterId(uint64_t value);

	public:
		/// Gets the id name.
		const std::string& name() const {
			return m_name;
		}

		/// Gets the id value.
		uint64_t value() const {
			return m_value;
		}

	private:
		std::string m_name;
		uint64_t m_value;
	};
}}
