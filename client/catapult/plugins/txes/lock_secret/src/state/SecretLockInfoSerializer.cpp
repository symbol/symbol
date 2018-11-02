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

#include "SecretLockInfoSerializer.h"

namespace catapult { namespace state {

	void SecretLockInfoExtendedDataSerializer::Save(const SecretLockInfo& lockInfo, io::OutputStream& output) {
		io::Write8(output, utils::to_underlying_type(lockInfo.HashAlgorithm));
		io::Write(output, lockInfo.Secret);
		io::Write(output, lockInfo.Recipient);
	}

	void SecretLockInfoExtendedDataSerializer::Load(io::InputStream& input, SecretLockInfo& lockInfo) {
		lockInfo.HashAlgorithm = static_cast<model::LockHashAlgorithm>(io::Read8(input));
		io::Read(input, lockInfo.Secret);
		io::Read(input, lockInfo.Recipient);
	}
}}
