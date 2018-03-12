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

	template<typename TTraits>
	void LockInfoCacheStorage<TTraits>::Save(const ValueType& pair, io::OutputStream& output) {
		const auto& lockInfo = pair.second;
		io::Write(output, lockInfo.Account);
		io::Write(output, lockInfo.MosaicId);
		io::Write(output, lockInfo.Amount);
		io::Write(output, lockInfo.Height);
		io::Write8(output, utils::to_underlying_type(lockInfo.Status));
		SaveLockInfo(lockInfo, output);
	}

	template<typename TTraits>
	void LockInfoCacheStorage<TTraits>::Load(io::InputStream& input, DestinationType& cacheDelta) {
		typename TTraits::ValueType lockInfo;
		io::Read(input, lockInfo.Account);
		io::Read(input, lockInfo.MosaicId);
		io::Read(input, lockInfo.Amount);
		io::Read(input, lockInfo.Height);
		lockInfo.Status = static_cast<model::LockStatus>(io::Read8(input));
		LoadLockInfo(input, lockInfo);

		cacheDelta.insert(lockInfo);
	}

	// explicit instantiation of both storage caches
	template struct LockInfoCacheStorage<HashLockInfoCacheDescriptor>;
	template struct LockInfoCacheStorage<SecretLockInfoCacheDescriptor>;
}}
