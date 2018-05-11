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

#include "LockInfoCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace cache {

	namespace {
		void SaveLockInfo(const model::HashLockInfo& value, io::OutputStream& output) {
			io::Write(output, value.Hash);
		}

		void LoadLockInfo(io::InputStream& input, model::HashLockInfo& value) {
			io::Read(input, value.Hash);
		}

		void SaveLockInfo(const model::SecretLockInfo& value, io::OutputStream& output) {
			io::Write8(output, utils::to_underlying_type(value.HashAlgorithm));
			io::Write(output, value.Secret);
			io::Write(output, value.Recipient);
		}

		void LoadLockInfo(io::InputStream& input, model::SecretLockInfo& value) {
			value.HashAlgorithm = static_cast<model::LockHashAlgorithm>(io::Read8(input));
			io::Read(input, value.Secret);
			io::Read(input, value.Recipient);
		}
	}

	template<typename TDescriptor>
	void LockInfoCacheStorage<TDescriptor>::Save(const StorageType& element, io::OutputStream& output) {
		const auto& lockInfo = element.second;
		io::Write(output, lockInfo.Account);
		io::Write(output, lockInfo.MosaicId);
		io::Write(output, lockInfo.Amount);
		io::Write(output, lockInfo.Height);
		io::Write8(output, utils::to_underlying_type(lockInfo.Status));
		SaveLockInfo(lockInfo, output);
	}

	template<typename TDescriptor>
	typename TDescriptor::ValueType LockInfoCacheStorage<TDescriptor>::Load(io::InputStream& input) {
		typename TDescriptor::ValueType lockInfo;
		io::Read(input, lockInfo.Account);
		io::Read(input, lockInfo.MosaicId);
		io::Read(input, lockInfo.Amount);
		io::Read(input, lockInfo.Height);
		lockInfo.Status = static_cast<model::LockStatus>(io::Read8(input));
		LoadLockInfo(input, lockInfo);
		return lockInfo;
	}

	template<typename TDescriptor>
	void LockInfoCacheStorage<TDescriptor>::LoadInto(io::InputStream& input, DestinationType& cacheDelta) {
		cacheDelta.insert(Load(input));
	}

	// explicit instantiation of both storage caches
	template struct LockInfoCacheStorage<HashLockInfoCacheDescriptor>;
	template struct LockInfoCacheStorage<SecretLockInfoCacheDescriptor>;
}}
