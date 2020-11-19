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
#include "RawFile.h"
#include <string>

namespace catapult { namespace io {

	/// Index file containing a uint64_t value.
	class IndexFile {
	public:
		/// Creates an index file with name \a filename and file locking specified by \a lockMode.
		explicit IndexFile(const std::string& filename, LockMode lockMode = LockMode::File);

	public:
		/// \c true if the index file exists.
		bool exists() const;

		/// Gets the index value.
		uint64_t get() const;

	public:
		/// Sets the index value to \a value.
		void set(uint64_t value);

		/// Increments the index value by one and returns the new value.
		uint64_t increment();

	private:
		RawFile open(OpenMode mode) const;

	private:
		std::string m_filename;
		LockMode m_lockMode;
	};
}}
