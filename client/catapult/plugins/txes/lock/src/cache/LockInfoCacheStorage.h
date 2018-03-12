#pragma once
#include "HashLockInfoCacheTypes.h"
#include "SecretLockInfoCacheTypes.h"
#include "catapult/cache/CacheStorageInclude.h"

namespace catapult { namespace cache {

	/// Policy for saving and loading lock info cache data.
	template<typename TDescriptor>
	struct LockInfoCacheStorage : public MapCacheStorageFromDescriptor<TDescriptor> {
	public:
		using typename MapCacheStorageFromDescriptor<TDescriptor>::ValueType;
		using typename MapCacheStorageFromDescriptor<TDescriptor>::DestinationType;

	public:
		/// Saves \a pair to \a output.
		static void Save(const ValueType& pair, io::OutputStream& output);

		/// Loads a single value from \a input into \a cacheDelta.
		static void Load(io::InputStream& input, DestinationType& cacheDelta);
	};

	extern template struct LockInfoCacheStorage<HashLockInfoCacheDescriptor>;
	using HashLockInfoCacheStorage = LockInfoCacheStorage<HashLockInfoCacheDescriptor>;

	extern template struct LockInfoCacheStorage<SecretLockInfoCacheDescriptor>;
	using SecretLockInfoCacheStorage = LockInfoCacheStorage<SecretLockInfoCacheDescriptor>;
}}
