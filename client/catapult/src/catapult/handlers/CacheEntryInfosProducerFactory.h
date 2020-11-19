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
#include "BasicProducer.h"
#include "HandlerTypes.h"
#include "catapult/cache/SynchronizedCache.h"
#include "catapult/model/CacheEntryInfo.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace handlers {

	/// Cache entry info producer.
	template<typename KeyType>
	using CacheEntryInfoProducer = supplier<std::shared_ptr<const model::CacheEntryInfo<KeyType>>>;

	/// Cache entry infos producer factory.
	template<typename TCacheDescriptor>
	class CacheEntryInfosProducerFactory {
	private:
		using KeyType = typename TCacheDescriptor::KeyType;
		using ValueType = typename TCacheDescriptor::ValueType;
		using CacheType = typename TCacheDescriptor::CacheType;

		class Producer : BasicProducer<model::EntityRange<KeyType>> {
		private:
			using ViewType = cache::LockedCacheView<typename TCacheDescriptor::CacheViewType>;
			using BasicProducer<model::EntityRange<KeyType>>::next;

		public:
			Producer(ViewType&& view, const model::EntityRange<KeyType>& keys)
					: BasicProducer<model::EntityRange<KeyType>>(keys)
					, m_pView(std::make_shared<ViewType>(std::move(view)))
			{}

		public:
			auto operator()() {
				return next([&view = **m_pView](const auto& key) {
					using Serializer = typename TCacheDescriptor::PatriciaTree::Serializer;

					auto iter = view.find(key);
					const auto* pEntry = iter.tryGetUnadapted();
					return pEntry ? MakeInfo(key, Serializer::SerializeValue(*pEntry)) : MakeInfo(key);
				});
			}

		private:
			static std::shared_ptr<model::CacheEntryInfo<KeyType>> MakeInfo(const KeyType& key) {
				auto pInfo = std::make_shared<model::CacheEntryInfo<KeyType>>();
				pInfo->Id = key;
				pInfo->DataSize = 0;
				pInfo->Size = static_cast<uint32_t>(model::CacheEntryInfo<KeyType>::CalculateRealSize(*pInfo));
				return pInfo;
			}

			static std::shared_ptr<model::CacheEntryInfo<KeyType>> MakeInfo(const KeyType& key, const std::string& value) {
				auto size = sizeof(model::CacheEntryInfo<KeyType>) + value.size();
				auto pInfo = utils::MakeSharedWithSize<model::CacheEntryInfo<KeyType>>(size);

				pInfo->Id = key;

				pInfo->DataSize = model::CacheEntryInfo<KeyType>::Max_Data_Size <= value.size()
						? model::CacheEntryInfo<KeyType>::Max_Data_Size
						: static_cast<uint32_t>(value.size());
				pInfo->Size = static_cast<uint32_t>(model::CacheEntryInfo<KeyType>::CalculateRealSize(*pInfo));
				std::memcpy(pInfo->DataPtr(), value.data(), value.size());
				return pInfo;
			}

		private:
			std::shared_ptr<ViewType> m_pView;
		};

	private:
		CacheEntryInfosProducerFactory(const CacheType& cache)
				: m_cache(cache)
		{}

	public:
		/// Creates a cache entry infos producer factory around \a cache.
		static CacheEntryInfosProducerFactory Create(const CacheType& cache) {
			return CacheEntryInfosProducerFactory(cache);
		}

	public:
		/// Creates a cache entry infos producer for a range of keys (\a ids).
		CacheEntryInfoProducer<KeyType> operator()(const model::EntityRange<KeyType>& ids) const {
			return Producer(m_cache.createView(), ids);
		}

	private:
		const CacheType& m_cache;
	};
}}
