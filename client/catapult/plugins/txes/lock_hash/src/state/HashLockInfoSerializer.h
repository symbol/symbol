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
#include "HashLockInfo.h"
#include "plugins/txes/lock_shared/src/state/LockInfoSerializer.h"

namespace catapult { namespace state {

	/// Policy for saving and loading hash lock info extended data.
	struct HashLockInfoExtendedDataSerializer {
		/// Saves \a lockInfo extended data to \a output.
		static void Save(const HashLockInfo& lockInfo, io::OutputStream& output);

		/// Loads hash lock info extended data from \a input into \a lockInfo.
		static void Load(io::InputStream& input, HashLockInfo& lockInfo);
	};

	/// Policy for saving and loading hash lock info data.
	struct HashLockInfoSerializer : public LockInfoSerializer<HashLockInfo, HashLockInfoExtendedDataSerializer> {
		/// Serialized state version.
		static constexpr uint16_t State_Version = 1;
	};
}}
