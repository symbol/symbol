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
#include "NonCopyable.h"
#include <iomanip>

namespace catapult { namespace utils {

	/// RAII class for modifying and restoring ostream formatting options.
	/// \note stream width is not sticky so it doesn't need to be saved and restored even if it is modified.
	class StreamFormatGuard : public NonCopyable {
	public:
		/// Creates a guard around \a out that sets format \a flags and character \a fill.
		StreamFormatGuard(std::ostream& out, std::ios_base::fmtflags flags, char fill)
				: m_out(out)
				, m_flags(m_out.flags(flags))
				, m_fill(m_out.fill(fill))
		{}

		/// Destroys the guard and restores original stream formatting settings.
		~StreamFormatGuard() {
			m_out.flags(m_flags);
			m_out.fill(m_fill);
		}

	private:
		std::ostream& m_out;
		std::ios_base::fmtflags m_flags;
		char m_fill;
	};
}}
