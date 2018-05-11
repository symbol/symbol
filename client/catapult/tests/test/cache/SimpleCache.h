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
#include "catapult/cache/CacheConfiguration.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/cache/SynchronizedCache.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "tests/test/nodeps/Atomics.h"
#include <numeric>

namespace catapult {
	namespace test {
		class BasicSimpleCacheDelta;
		class BasicSimpleCacheView;
	}
}

namespace catapult { namespace test {

	using SimpleCacheReadOnlyType = cache::ReadOnlySimpleCache<BasicSimpleCacheView, BasicSimpleCacheDelta, size_t>;

	/// Possible simple cache view modes.
	enum class SimpleCacheViewMode {
		/// View supports iteration.
		Iterable,
		/// View does not support iteration.
		Non_Iterable
	};

	// region SimpleCacheView

	/// Basic view on top of the simple cache.
	/// \note size and contains are provided for compatibility with ReadOnlySimpleCache.
	/// \note get and isActive are provided for compatibility with ReadOnlyArtifactCache.
	/// \note cbegin and cend are provided for compatibility with CacheStorageAdapter.
	class BasicSimpleCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = SimpleCacheReadOnlyType;

	public:
		/// Creates a view around \a mode and \a id.
		explicit BasicSimpleCacheView(SimpleCacheViewMode mode, const size_t& id)
				: m_mode(mode)
				, m_id(id)
				, m_ids(id) {
			std::iota(m_ids.begin(), m_ids.end(), 1);
		}

	public:
		/// Gets the id.
		size_t id() const {
			return m_id;
		}

	public:
		/// Gets the size.
		size_t size() const {
			return m_id;
		}

		/// Returns \c true if \a id is contained by this view.
		bool contains(size_t id) const {
			return id <= m_id;
		}

	public:
		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		size_t get(size_t id) const {
			if (!contains(id))
				CATAPULT_THROW_OUT_OF_RANGE("invalid id supplied to get");

			return id * id;
		}

		/// Gets a value indicating whether or not an artifact with \a id is active at \a height.
		bool isActive(size_t id, Height height) const {
			return contains(id) && 0 == id % height.unwrap();
		}

	public:
		/// Returns a const iterator to the first cache element.
		auto begin() const {
			return m_ids.cbegin();
		}

		/// Returns a const iterator to the element following the last cache element.
		auto end() const {
			return m_ids.cend();
		}

	public:
		/// Makes an iterable view of this cache.
		const auto* tryMakeIterableView() const {
			return SimpleCacheViewMode::Iterable == m_mode ? this : nullptr;
		}

	private:
		SimpleCacheViewMode m_mode;
		const size_t& m_id;
		std::vector<size_t> m_ids;
	};

	/// View on top of the simple cache.
	class SimpleCacheView : public cache::ReadOnlyViewSupplier<BasicSimpleCacheView> {
	public:
		/// Creates a view around \a id.
		/// \note This overload is needed for ReadOnlyViewSupplier tests.
		explicit SimpleCacheView(const size_t& id) : ReadOnlyViewSupplier(SimpleCacheViewMode::Iterable, id)
		{}

		/// Creates a view around \a mode and \a id.
		SimpleCacheView(SimpleCacheViewMode mode, const size_t& id) : ReadOnlyViewSupplier(mode, id)
		{}
	};

	// endregion

	// region SimpleCacheDelta

	/// Basic delta on top of the simple cache.
	/// \note size and contains are provided for compatibility with ReadOnlySimpleCache.
	/// \note get and isActive are provided for compatibility with ReadOnlyArtifactCache.
	class BasicSimpleCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = SimpleCacheReadOnlyType;

	public:
		/// Creates a delta around \a id.
		explicit BasicSimpleCacheDelta(size_t id) : m_id(id)
		{}

	public:
		/// Gets the id.
		size_t id() const {
			return m_id;
		}

		/// Increments the id.
		void increment() {
			++m_id;
		}

	public:
		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		size_t get(size_t id) const {
			if (!contains(id))
				CATAPULT_THROW_OUT_OF_RANGE("invalid id supplied to get");

			return id * id;
		}

		/// Gets a value indicating whether or not an artifact with \a id is active at \a height.
		bool isActive(size_t id, Height height) const {
			return contains(id) && 0 == id % height.unwrap();
		}

	public:
		/// Gets the size.
		size_t size() const {
			return m_id;
		}

		/// Returns \c true if \a id is contained by this view.
		bool contains(size_t id) const {
			return id <= m_id;
		}

		/// Inserts \a id into the cache.
		void insert(size_t id) {
			m_id = id;
		}

	private:
		size_t m_id;
	};

	/// Delta on top of the simple cache.
	class SimpleCacheDelta : public cache::ReadOnlyViewSupplier<BasicSimpleCacheDelta> {
	public:
		/// Creates a delta around \a id.
		explicit SimpleCacheDelta(size_t id) : ReadOnlyViewSupplier(id)
		{}
	};

	// endregion

	// region SimpleCache

	/// Cache composed of simple data.
	class BasicSimpleCache : public utils::MoveOnly {
	public:
		using CacheViewType = SimpleCacheView;
		using CacheDeltaType = SimpleCacheDelta;
		using CacheReadOnlyType = SimpleCacheReadOnlyType;

	public:
		/// Creates a cache with an optional auto set flag (\a pFlag) and view \a mode.
		explicit BasicSimpleCache(
				const std::shared_ptr<const test::AutoSetFlag::State>& pFlag = nullptr,
				SimpleCacheViewMode mode = SimpleCacheViewMode::Iterable)
				: m_pFlag(pFlag)
				, m_mode(mode)
				, m_id(0)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_mode, m_id);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return CacheDeltaType(m_id);
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return CacheDeltaType(m_id);
		}

		/// Commits all pending changes in \a delta to the underlying storage.
		void commit(const CacheDeltaType& delta) {
			if (m_pFlag) {
				CATAPULT_LOG(debug) << "blocking in commit using auto set flag (" << m_pFlag << ")";
				m_pFlag->wait();
			}

			m_id = delta.id();
		}

	private:
		std::shared_ptr<const test::AutoSetFlag::State> m_pFlag;
		SimpleCacheViewMode m_mode;
		size_t m_id;
	};

	/// Synchronized cache composed of simple data.
	class SimpleCache : public cache::SynchronizedCache<BasicSimpleCache> {
	public:
		/// Cache friendly name.
		static constexpr auto Name = "SimpleCache";

	public:
		/// Creates a cache around \a config.
		SimpleCache(const cache::CacheConfiguration& = cache::CacheConfiguration()) : SynchronizedCache(BasicSimpleCache())
		{}

		/// Creates a cache with \a mode.
		explicit SimpleCache(SimpleCacheViewMode mode) : SynchronizedCache(BasicSimpleCache(nullptr, mode))
		{}

		/// Creates a cache with an auto set \a flag.
		explicit SimpleCache(const test::AutoSetFlag& flag) : SynchronizedCache(BasicSimpleCache(flag.state())) {
			CATAPULT_LOG(debug) << "created SimpleCache with auto set flag (" << &flag << ") with state " << flag.state()->isSet();
		}
	};

	/// Synchronized cache composed of simple data with a specific id.
	template<size_t CacheId>
	class SimpleCacheT : public SimpleCache {
	public:
		/// Unique cache identifier.
		static constexpr size_t Id = CacheId;

	public:
		using SimpleCache::SimpleCache;
	};

	// endregion

	// region SimpleCacheStorageTraits

	/// Policy for saving and loading simple cache data.
	struct SimpleCacheStorageTraits {
		using SourceType = SimpleCacheView;
		using DestinationType = SimpleCacheDelta;

		/// Saves \a value to \a output.
		static void Save(size_t value, io::OutputStream& output) {
			// Act: encode each value before writing
			io::Write64(output, value ^ 0xFFFFFFFF'FFFFFFFFull);
		}

		/// Loads a single value from \a input into \a cacheDelta.
		static void LoadInto(io::InputStream& input, DestinationType& cacheDelta) {
			// Act: decode each value after reading (and ensure the expected values are read)
			auto value = io::Read64(input) ^ 0xFFFFFFFF'FFFFFFFFull;
			if (value - 1 != cacheDelta.id())
				CATAPULT_THROW_RUNTIME_ERROR_2("read value was unexpected (value, id)", value, cacheDelta.id());

			cacheDelta.increment();
		}
	};

	// endregion
}}
