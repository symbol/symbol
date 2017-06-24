#pragma once
#include "CacheStorage.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/Traits.h"
#include <functional>
#include <vector>

namespace catapult { namespace cache {

	/// Saves \a source data to \a output.
	template<typename TStorageTraits>
	void SaveAllData(const typename TStorageTraits::SourceType& source, io::OutputStream& output) {
		io::Write(output, static_cast<uint64_t>(source.size()));
		for (auto iter = source.cbegin(); source.cend() != iter; ++iter)
			TStorageTraits::Save(*iter, output);

		output.flush();
	}

	/// Loads data from an input stream in chunks.
	template<typename TStorageTraits>
	class ChunkedDataLoader {
	private:
		using LoaderFunc = std::function<void (io::InputStream&, typename TStorageTraits::DestinationType&)>;

		enum class LoaderType { Basic, Stateful, CacheDependent };
		using BasicLoaderFlag = std::integral_constant<LoaderType, LoaderType::Basic>;
		using StatefulLoaderFlag = std::integral_constant<LoaderType, LoaderType::Stateful>;
		using CacheDependentLoaderFlag = std::integral_constant<LoaderType, LoaderType::CacheDependent>;

	public:
		/// Creates a chunked loader around \a input and \a catapultCache.
		explicit ChunkedDataLoader(io::InputStream& input, const cache::CatapultCache& catapultCache)
				: m_input(input)
				, m_loader(CreateLoader(LoadStateAccessor<TStorageTraits>(), catapultCache)) {
			io::Read(input, m_numRemainingEntries);
		}

	public:
		/// Returns \c true if there are more entries in the input.
		bool hasNext() const {
			return 0 != m_numRemainingEntries;
		}

		/// Loads the next data chunk of at most \a numRequestedEntries into \a destination.
		void next(uint64_t numRequestedEntries, typename TStorageTraits::DestinationType& destination) {
			numRequestedEntries = std::min(numRequestedEntries, m_numRemainingEntries);
			m_numRemainingEntries -= numRequestedEntries;
			while (numRequestedEntries--)
				m_loader(m_input, destination);
		}

	private:
		template<typename T, typename = void>
		struct LoadStateAccessor
				: BasicLoaderFlag
		{};

		template<typename T>
		struct LoadStateAccessor<T, typename utils::traits::enable_if_type<typename T::LoadStateType>::type>
				: StatefulLoaderFlag
		{};

		template<typename T>
		struct LoadStateAccessor<T, typename utils::traits::enable_if_type<typename T::DependencyCacheType>::type>
				: CacheDependentLoaderFlag
		{};

	private:
		static LoaderFunc CreateLoader(BasicLoaderFlag, const cache::CatapultCache&) {
			return TStorageTraits::Load;
		}

		static LoaderFunc CreateLoader(StatefulLoaderFlag, const cache::CatapultCache&) {
			return [state = typename TStorageTraits::LoadStateType()](auto& input, auto& destination) mutable {
				return TStorageTraits::Load(input, destination, state);
			};
		}

		static LoaderFunc CreateLoader(CacheDependentLoaderFlag, const cache::CatapultCache& catapultCache) {
			return [&catapultCache](auto& input, auto& destination) {
				auto dependencyCacheView = catapultCache.sub<typename TStorageTraits::DependencyCacheType>().createView();
				return TStorageTraits::Load(input, destination, *dependencyCacheView);
			};
		}

	private:
		io::InputStream& m_input;
		uint64_t m_numRemainingEntries;
		LoaderFunc m_loader;
	};

	/// A CacheStorage implementation that wraps a cache and associated storage traits.
	template<typename TCache, typename TStorageTraits>
	class CacheStorageAdapter : public CacheStorage {
	public:
		/// Creates an adapter around \a cache and \a catapultCache.
		explicit CacheStorageAdapter(TCache& cache, const cache::CatapultCache& catapultCache)
				: m_cache(cache)
				, m_catapultCache(catapultCache)
				, m_name(TCache::Name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

	public:
		void saveAll(io::OutputStream& output) const override {
			auto view = m_cache.createView();
			SaveAllData<TStorageTraits>(*view, output);
		}

		void loadAll(io::InputStream& input, size_t batchSize) override {
			auto delta = m_cache.createDelta();

			ChunkedDataLoader<TStorageTraits> loader(input, m_catapultCache);
			while (loader.hasNext()) {
				loader.next(batchSize, *delta);
				m_cache.commit();
			}
		}

	private:
		TCache& m_cache;
		const cache::CatapultCache& m_catapultCache;
		std::string m_name;
	};
}}
