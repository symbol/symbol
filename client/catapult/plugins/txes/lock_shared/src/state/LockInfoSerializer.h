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
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	/// Policy for saving and loading lock info data.
	template<typename TLockInfo, typename TLockInfoSerializer>
	struct LockInfoSerializer {
	public:
		/// Saves \a lockInfo to \a output.
		static void Save(const TLockInfo& lockInfo, io::OutputStream& output){
			output.write(lockInfo.OwnerAddress);
			io::Write(output, lockInfo.MosaicId);
			io::Write(output, lockInfo.Amount);
			io::Write(output, lockInfo.EndHeight);
			io::Write8(output, utils::to_underlying_type(lockInfo.Status));
			TLockInfoSerializer::Save(lockInfo, output);
		}

		/// Loads a single value from \a input.
		static TLockInfo Load(io::InputStream& input){
			TLockInfo lockInfo;
			input.read(lockInfo.OwnerAddress);
			io::Read(input, lockInfo.MosaicId);
			io::Read(input, lockInfo.Amount);
			io::Read(input, lockInfo.EndHeight);
			lockInfo.Status = static_cast<LockStatus>(io::Read8(input));
			TLockInfoSerializer::Load(input, lockInfo);
			return lockInfo;
		}
	};
}}
