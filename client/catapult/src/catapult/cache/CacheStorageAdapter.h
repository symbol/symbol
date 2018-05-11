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
#include "CacheStorage.h"
#include "ChunkedDataLoader.h"

namespace catapult { namespace cache {

	/// A CacheStorage implementation that wraps a cache and associated storage traits.
	template<typename TCache, typename TStorageTraits>
	class CacheStorageAdapter : public CacheStorage {
	public:
		/// Creates an adapter around \a cache.
		explicit CacheStorageAdapter(TCache& cache)
				: m_cache(cache)
				, m_name(TCache::Name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

	public:
		void saveAll(io::OutputStream& output) const override {
			auto view = m_cache.createView();
			io::Write64(output, view->size());

			auto pIterableView = view->tryMakeIterableView();
			for (const auto& value : *pIterableView)
				TStorageTraits::Save(value, output);

			output.flush();
		}

		void loadAll(io::InputStream& input, size_t batchSize) override {
			auto delta = m_cache.createDelta();

			ChunkedDataLoader<TStorageTraits> loader(input);
			while (loader.hasNext()) {
				loader.next(batchSize, *delta);
				m_cache.commit();
			}
		}

	private:
		TCache& m_cache;
		std::string m_name;
	};
}}
