#pragma once
#include "CacheStorageInclude.h"
#include <string>

namespace catapult { namespace cache {

	/// Interface for loading and saving cache data.
	class CacheStorage {
	public:
		virtual ~CacheStorage() {}

	public:
		/// Gets the cache name.
		virtual const std::string& name() const = 0;

	public:
		/// Saves cache data to \a output.
		virtual void saveAll(io::OutputStream& output) const = 0;

		/// Loads cache data from \a input in batches of \a batchSize.
		virtual void loadAll(io::InputStream& input, size_t batchSize) = 0;
	};
}}
