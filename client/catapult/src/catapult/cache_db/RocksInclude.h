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

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4100) /* unreferenced formal parameter */
#endif

#include <rocksdb/compaction_filter.h>
#include <rocksdb/db.h>
#include <rocksdb/write_batch.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace catapult { namespace cache {

	/// Maximum length of special keys that should not be pruned.
	/// \note Value should be <= sizeof(uint64_t)
	constexpr size_t Special_Key_Max_Length = 8;
}}
