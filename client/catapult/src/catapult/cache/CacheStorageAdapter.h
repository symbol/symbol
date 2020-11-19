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
#include "CacheStorage.h"
#include "CatapultCacheView.h"
#include "ChunkedDataLoader.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	/// CacheStorage implementation that wraps a cache and associated storage traits.
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
		void saveAll(const CatapultCacheView& cacheView, io::OutputStream& output) const override {
			const auto& view = cacheView.sub<TCache>();
			io::Write64(output, view.size());

			auto pIterableView = view.tryMakeIterableView();
			for (const auto& element : *pIterableView)
				SaveValue(element, output);

			output.flush();
		}

		void saveSummary(const CatapultCacheDelta&, io::OutputStream&) const override {
			CATAPULT_THROW_INVALID_ARGUMENT("CacheStorageAdapter does not support saveSummary");
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
		// assume pair indicates maps and only forward value to save

		template<typename T>
		static void SaveValue(const T& value, io::OutputStream& output) {
			TStorageTraits::Save(value, output);
		}

		template<typename T1, typename T2>
		static void SaveValue(const std::pair<T1, T2>& pair, io::OutputStream& output) {
			TStorageTraits::Save(pair.second, output);
		}

	private:
		TCache& m_cache;
		std::string m_name;
	};
}}
