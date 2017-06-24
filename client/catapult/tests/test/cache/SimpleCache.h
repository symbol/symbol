#pragma once
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

	// region SimpleCacheView

	/// Basic view on top of the simple cache.
	/// \note size and contains are provided for compatibility with ReadOnlySimpleCache.
	/// \note get and isActive are provided for compatibility with ReadOnlyArtifactCache.
	/// \note cbegin and cend are provided for compatibility with CacheStorageAdapter.
	class BasicSimpleCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = SimpleCacheReadOnlyType;

	public:
		/// Creates a view around \a id.
		explicit BasicSimpleCacheView(const size_t& id) : m_id(id), m_ids(id) {
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
		auto cbegin() const {
			return m_ids.cbegin();
		}

		/// Returns a const iterator to the element following the last cache element.
		auto cend() const {
			return m_ids.cend();
		}

	private:
		const size_t& m_id;
		std::vector<size_t> m_ids;
	};

	/// View on top of the simple cache.
	class SimpleCacheView : public cache::ReadOnlyViewSupplier<BasicSimpleCacheView> {
	public:
		/// Creates a view around \a id.
		explicit SimpleCacheView(const size_t& id) : ReadOnlyViewSupplier(id)
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
		/// Creates a cache with an optional auto set flag (\a pFlag).
		explicit BasicSimpleCache(const test::AutoSetFlag* pFlag)
				: m_id(0)
				, m_pFlag(pFlag)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_id);
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
			if (m_pFlag)
				m_pFlag->wait();

			m_id = delta.id();
		}

	private:
		size_t m_id;
		const test::AutoSetFlag* m_pFlag;
	};

	/// Synchronized cache composed of simple data.
	class SimpleCache : public cache::SynchronizedCache<BasicSimpleCache> {
	public:
		/// The cache friendly name.
		static constexpr auto Name = "SimpleCache";

	public:
		/// Creates a cache with an optional auto set flag (\a pFlag).
		explicit SimpleCache(const test::AutoSetFlag* pFlag = nullptr) : SynchronizedCache(BasicSimpleCache(pFlag))
		{}
	};

	/// Synchronized cache composed of simple data with a specific id.
	template<size_t CacheId>
	class SimpleCacheT : public SimpleCache {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = CacheId;
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
			io::Write(output, value ^ 0xFFFFFFFF);
		}

		/// Loads a single value from \a input into \a cacheDelta.
		static void Load(io::InputStream& input, DestinationType& cacheDelta) {
			// Act: decode each value after reading (and ensure the expected values are read)
			auto value = io::Read<uint64_t>(input) ^ 0xFFFFFFFF;
			if (value - 1 != cacheDelta.id())
				CATAPULT_THROW_RUNTIME_ERROR_2("read value was unexpected (value, id)", value, cacheDelta.id());

			cacheDelta.increment();
		}
	};

	// endregion
}}
