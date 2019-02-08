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
#include "catapult/tree/TreeNode.h"
#include "tests/test/nodeps/Atomics.h"
#include <numeric>

namespace catapult {
	namespace test {
		template<typename TViewExtension, typename TDeltaExtension>
		class BasicSimpleCacheDeltaExtension;

		template<typename TViewExtension, typename TDeltaExtension>
		class BasicSimpleCacheViewExtension;
	}
}

namespace catapult { namespace test {

	template<typename TViewExtension, typename TDeltaExtension>
	using SimpleCacheReadOnlyType = cache::ReadOnlySimpleCache<
		BasicSimpleCacheViewExtension<TViewExtension, TDeltaExtension>,
		BasicSimpleCacheDeltaExtension<TViewExtension, TDeltaExtension>,
		size_t>;

	/// Possible simple cache view modes.
	enum class SimpleCacheViewMode {
		/// View supports iteration.
		Iterable,
		/// View supports merkle roots.
		Merkle_Root,
		/// View only supports minimum functionality.
		Basic
	};

	/// Find iterator returned by simple cache views.
	/// \note Template on a tag so that view and delta return different iterator types to better emulate real caches.
	template<size_t Tag>
	class SimpleCacheFindIterator {
	public:
		/// Creates an uninitialized iterator.
		SimpleCacheFindIterator() = default;

		/// Creates an iterator around \a value and \a isValid.
		SimpleCacheFindIterator(size_t value, bool isValid)
				: m_value(value)
				, m_isValid(isValid)
		{}

	public:
		/// Gets a value.
		/// \throws catapult_out_of_range if this iterator does not point to a valid value.
		const size_t& get() const {
			if (!m_isValid)
				CATAPULT_THROW_OUT_OF_RANGE("invalid id supplied to get");

			return m_value;
		}

		/// Tries to get a const (unadapted) value.
		const auto* tryGetUnadapted() const {
			return &get();
		}

	private:
		size_t m_value;
		bool m_isValid;
	};

	/// Simple cache state that is passed down from cache to views.
	struct SimpleCacheState {
	public:
		/// Creates default state.
		SimpleCacheState()
				: Id(0)
				, MerkleRoot(GenerateRandomData<Hash256_Size>())
		{}

	public:
		/// Current cache identifier / value.
		size_t Id;

		/// Cache merkle root.
		Hash256 MerkleRoot;
	};

	// region view extensions

	/// A view extension that does not support merkle roots.
	class SimpleCacheDisabledMerkleRootViewExtension {
	public:
		/// Creates a view extension.
		explicit SimpleCacheDisabledMerkleRootViewExtension(SimpleCacheViewMode, const SimpleCacheState&)
		{}
	};

	/// A view extension that represents a default cache that supports merkle roots.
	class SimpleCacheDefaultViewExtension {
	public:
		/// Creates a view extension around \a mode and \a state.
		explicit SimpleCacheDefaultViewExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: SimpleCacheDefaultViewExtension(mode, state.MerkleRoot)
		{}

		/// Creates a view extension around \a mode and \a merkleRoot.
		explicit SimpleCacheDefaultViewExtension(SimpleCacheViewMode mode, const Hash256& merkleRoot)
				: m_mode(mode)
				, m_merkleRoot(merkleRoot)
		{}

	public:
		/// Returns \c true if merkle root is supported.
		bool supportsMerkleRoot() const {
			return SimpleCacheViewMode::Merkle_Root == m_mode;
		}

		/// Tries to get the merkle root if supported.
		std::pair<Hash256, bool> tryGetMerkleRoot() const {
			return std::make_pair(m_merkleRoot, supportsMerkleRoot());
		}

		/// Tries to find the value associated with (key) in the tree and stores proof of existence or not in (nodePath).
		/// \note This is just a placeholder and not implemented.
		std::pair<Hash256, bool> tryLookup(const size_t&, std::vector<tree::TreeNode>&) const {
			return std::make_pair(Hash256(), false);
		}

	private:
		SimpleCacheViewMode m_mode;
		const Hash256& m_merkleRoot;
	};

	/// A delta extension that represents a default cache that supports merkle roots.
	class SimpleCacheDefaultDeltaExtension : public SimpleCacheDefaultViewExtension {
	public:
		/// Creates a delta extension around \a mode and \a state.
		explicit SimpleCacheDefaultDeltaExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: SimpleCacheDefaultDeltaExtension(mode, std::make_unique<Hash256>(state.MerkleRoot))
		{}

	private:
		explicit SimpleCacheDefaultDeltaExtension(SimpleCacheViewMode mode, std::unique_ptr<Hash256>&& pMerkleRoot)
				: SimpleCacheDefaultViewExtension(mode, *pMerkleRoot)
				, m_pMerkleRoot(std::move(pMerkleRoot))
		{}

	public:
		/// Recalculates the merkle root given the specified chain \a height if supported.
		void updateMerkleRoot(Height height) {
			// change the first byte
			(*m_pMerkleRoot)[0] = static_cast<uint8_t>(height.unwrap());
		}

		/// Sets the merkle root (\a merkleRoot) if supported.
		/// \note There must not be any pending changes.
		void setMerkleRoot(const Hash256& merkleRoot) {
			*m_pMerkleRoot = merkleRoot;
		}

	private:
		std::unique_ptr<Hash256> m_pMerkleRoot;
	};

	// endregion

	// region SimpleCacheView

	/// Basic view on top of the simple cache.
	/// \note size and contains are provided for compatibility with ReadOnlySimpleCache.
	/// \note get and isActive are provided for compatibility with ReadOnlyArtifactCache.
	/// \note cbegin and cend are provided for compatibility with CacheStorageAdapter.
	template<typename TViewExtension, typename TDeltaExtension>
	class BasicSimpleCacheViewExtension
			: public TViewExtension
			, public utils::MoveOnly {
	public:
		using ReadOnlyView = SimpleCacheReadOnlyType<TViewExtension, TDeltaExtension>;

		using const_iterator = SimpleCacheFindIterator<0>;

	public:
		/// Creates a view around \a mode and \a state.
		explicit BasicSimpleCacheViewExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: TViewExtension(mode, state)
				, m_mode(mode)
				, m_id(state.Id)
				, m_ids(m_id) {
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
		/// Finds the cache value identified by \a id.
		const_iterator find(size_t id) const {
			return const_iterator(id * id, contains(id));
		}

		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		size_t get(size_t id) const {
			return find(id).get();
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
	template<typename TViewExtension, typename TDeltaExtension>
	class SimpleCacheViewExtension : public cache::ReadOnlyViewSupplier<BasicSimpleCacheViewExtension<TViewExtension, TDeltaExtension>> {
	public:
		/// Creates a view around \a mode and \a state.
		SimpleCacheViewExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: cache::ReadOnlyViewSupplier<BasicSimpleCacheViewExtension<TViewExtension, TDeltaExtension>>(mode, state)
		{}
	};

	using BasicSimpleCacheView = BasicSimpleCacheViewExtension<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;
	using SimpleCacheView = SimpleCacheViewExtension<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;

	// endregion

	// region SimpleCacheDelta

	/// Basic delta on top of the simple cache.
	/// \note size and contains are provided for compatibility with ReadOnlySimpleCache.
	/// \note get and isActive are provided for compatibility with ReadOnlyArtifactCache.
	template<typename TViewExtension, typename TDeltaExtension>
	class BasicSimpleCacheDeltaExtension
			: public TDeltaExtension
			, public utils::MoveOnly {
	public:
		using ReadOnlyView = SimpleCacheReadOnlyType<TViewExtension, TDeltaExtension>;

		using const_iterator = SimpleCacheFindIterator<1>;

	public:
		/// Creates a view around \a mode and \a state.
		explicit BasicSimpleCacheDeltaExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: TDeltaExtension(mode, state)
				, m_id(state.Id)
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
		/// Finds the cache value identified by \a id.
		const_iterator find(size_t id) const {
			return const_iterator(id * id, contains(id));
		}

		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		size_t get(size_t id) const {
			return find(id).get();
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
	template<typename TViewExtension, typename TDeltaExtension>
	class SimpleCacheDeltaExtension : public cache::ReadOnlyViewSupplier<BasicSimpleCacheDeltaExtension<TViewExtension, TDeltaExtension>> {
	public:
		/// Creates a delta around \a mode and \a state.
		SimpleCacheDeltaExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: cache::ReadOnlyViewSupplier<BasicSimpleCacheDeltaExtension<TViewExtension, TDeltaExtension>>(mode, state)
		{}
	};

	using BasicSimpleCacheDelta = BasicSimpleCacheDeltaExtension<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;
	using SimpleCacheDelta = SimpleCacheDeltaExtension<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;

	// endregion

	// region SimpleCache

	/// Cache composed of simple data.
	template<typename TViewExtension, typename TDeltaExtension>
	class BasicSimpleCacheExtension : public utils::MoveOnly {
	public:
		using CacheViewType = SimpleCacheViewExtension<TViewExtension, TDeltaExtension>;
		using CacheDeltaType = SimpleCacheDeltaExtension<TViewExtension, TDeltaExtension>;
		using CacheReadOnlyType = SimpleCacheReadOnlyType<TViewExtension, TDeltaExtension>;

	public:
		/// Creates a cache with an optional auto set flag (\a pFlag) and view \a mode.
		explicit BasicSimpleCacheExtension(
				const std::shared_ptr<const AutoSetFlag::State>& pFlag = nullptr,
				SimpleCacheViewMode mode = SimpleCacheViewMode::Iterable)
				: m_pFlag(pFlag)
				, m_mode(mode)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_mode, m_state);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return CacheDeltaType(m_mode, m_state);
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return CacheDeltaType(m_mode, m_state);
		}

		/// Commits all pending changes in \a delta to the underlying storage.
		void commit(const CacheDeltaType& delta) {
			if (m_pFlag) {
				CATAPULT_LOG(debug) << "blocking in commit using auto set flag (" << m_pFlag << ")";
				m_pFlag->wait();
			}

			m_state.Id = delta.id();
		}

	private:
		std::shared_ptr<const AutoSetFlag::State> m_pFlag;
		SimpleCacheViewMode m_mode;
		SimpleCacheState m_state;
	};

	/// Synchronized cache composed of simple data.
	template<typename TViewExtension, typename TDeltaExtension>
	class SimpleCacheExtension : public cache::SynchronizedCache<BasicSimpleCacheExtension<TViewExtension, TDeltaExtension>> {
	private:
		using BaseType = cache::SynchronizedCache<BasicSimpleCacheExtension<TViewExtension, TDeltaExtension>>;

	public:
		/// Cache friendly name.
		static constexpr auto Name = "SimpleCache";

	public:
		/// Creates a cache around \a config.
		explicit SimpleCacheExtension(const cache::CacheConfiguration& = cache::CacheConfiguration())
				: BaseType(BasicSimpleCacheExtension<TViewExtension, TDeltaExtension>())
		{}

		/// Creates a cache with \a mode.
		explicit SimpleCacheExtension(SimpleCacheViewMode mode)
				: BaseType(BasicSimpleCacheExtension<TViewExtension, TDeltaExtension>(nullptr, mode))
		{}

		/// Creates a cache with an auto set \a flag.
		explicit SimpleCacheExtension(const AutoSetFlag& flag)
				: BaseType(BasicSimpleCacheExtension<TViewExtension, TDeltaExtension>(flag.state())) {
			CATAPULT_LOG(debug) << "created SimpleCache with auto set flag (" << &flag << ") with state " << flag.state()->isSet();
		}
	};

	using BasicSimpleCache = BasicSimpleCacheExtension<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;
	using SimpleCache = SimpleCacheExtension<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;

	/// Synchronized cache composed of simple data with a specific id.
	template<
		size_t CacheId,
		typename TViewExtension = SimpleCacheDefaultViewExtension,
		typename TDeltaExtension = SimpleCacheDefaultDeltaExtension
	>
	class SimpleCacheT : public SimpleCacheExtension<TViewExtension, TDeltaExtension> {
	public:
		/// Unique cache identifier.
		static constexpr size_t Id = CacheId;

	public:
		using SimpleCacheExtension<TViewExtension, TDeltaExtension>::SimpleCacheExtension;
	};

	// endregion

	// region SimpleCacheStorageTraits

	/// Policy for saving and loading simple cache data.
	template<typename TViewExtension, typename TDeltaExtension>
	struct SimpleCacheExtensionStorageTraits {
		using SourceType = SimpleCacheViewExtension<TViewExtension, TDeltaExtension>;
		using DestinationType = SimpleCacheDeltaExtension<TViewExtension, TDeltaExtension>;

		/// Saves \a value to \a output.
		static void Save(size_t value, io::OutputStream& output) {
			// Act: encode each value before writing
			io::Write64(output, value ^ 0xFFFFFFFF'FFFFFFFFull);
		}

		/// Loads a single value from \a input.
		static uint64_t Load(io::InputStream& input) {
			// Act: decode each value after reading
			return io::Read64(input) ^ 0xFFFFFFFF'FFFFFFFFull;
		}

		/// Loads \a value into \a cacheDelta.
		static void LoadInto(uint64_t value, DestinationType& cacheDelta) {
			// Assert: the expected values are read
			if (value - 1 != cacheDelta.id())
				CATAPULT_THROW_RUNTIME_ERROR_2("read value was unexpected (value, id)", value, cacheDelta.id());

			cacheDelta.increment();
		}
	};

	using SimpleCacheStorageTraits = SimpleCacheExtensionStorageTraits<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;

	// endregion
}}
