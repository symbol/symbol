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
#include "SecretLockInfoHistory.h"
#include "plugins/txes/lock_shared/src/state/LockInfoHistorySerializer.h"

namespace catapult { namespace state {

	/// Policy for saving and loading secret lock info extended data.
	struct SecretLockInfoExtendedDataSerializer {
		/// Saves \a lockInfo extended data to \a output.
		static void Save(const SecretLockInfo& lockInfo, io::OutputStream& output);

		/// Loads secret lock info extended data from \a input into \a lockInfo.
		static void Load(io::InputStream& input, SecretLockInfo& lockInfo);
	};

	DEFINE_LOCK_INFO_HISTORY_SERIALIZERS(SecretLockInfo, 1)
}}
