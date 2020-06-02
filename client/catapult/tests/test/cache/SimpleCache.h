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
#include <unordered_set>

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
		uint64_t>;

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
	template<uint64_t Tag>
	class SimpleCacheFindIterator {
	public:
		/// Creates an uninitialized iterator.
		SimpleCacheFindIterator() = default;

		/// Creates an iterator around \a value and \a isValid.
		SimpleCacheFindIterator(uint64_t value, bool isValid)
				: m_value(value)
				, m_isValid(isValid)
		{}

	public:
		/// Gets a value.
		/// \throws catapult_out_of_range if this iterator does not point to a valid value.
		const uint64_t& get() const {
			if (!m_isValid)
				CATAPULT_THROW_OUT_OF_RANGE("invalid id supplied to get");

			return m_value;
		}

		/// Tries to get a const (unadapted) value.
		const auto* tryGetUnadapted() const {
			return &get();
		}

	private:
		uint64_t m_value;
		bool m_isValid;
	};

	/// Simple cache state that is passed down from cache to views.
	struct SimpleCacheState {
	public:
		/// Creates default state.
		SimpleCacheState()
				: Id(0)
				, MerkleRoot(GenerateRandomByteArray<Hash256>())
		{}

	public:
		/// Current cache identifier / value.
		uint64_t Id;

		/// Cache merkle root.
		Hash256 MerkleRoot;
	};

	// region view extensions

	/// View extension that does not support merkle roots.
	class SimpleCacheDisabledMerkleRootViewExtension {
	public:
		/// Creates a view extension.
		SimpleCacheDisabledMerkleRootViewExtension(SimpleCacheViewMode, const SimpleCacheState&)
		{}
	};

	/// View extension that represents a default cache that supports optional features (merkle roots).
	class SimpleCacheDefaultViewExtension {
	public:
		/// Creates a view extension around \a mode and \a state.
		SimpleCacheDefaultViewExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: SimpleCacheDefaultViewExtension(mode, state.MerkleRoot)
		{}

		/// Creates a view extension around \a mode and \a merkleRoot.
		SimpleCacheDefaultViewExtension(SimpleCacheViewMode mode, const Hash256& merkleRoot)
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
		std::pair<Hash256, bool> tryLookup(const uint64_t&, std::vector<tree::TreeNode>&) const {
			return std::make_pair(Hash256(), false);
		}

	private:
		SimpleCacheViewMode m_mode;
		const Hash256& m_merkleRoot;
	};

	/// Delta extension that represents a default cache that supports optional features (merkle roots and pruning).
	class SimpleCacheDefaultDeltaExtension : public SimpleCacheDefaultViewExtension {
	public:
		/// Creates a delta extension around \a mode and \a state.
		SimpleCacheDefaultDeltaExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: SimpleCacheDefaultDeltaExtension(mode, std::make_unique<Hash256>(state.MerkleRoot))
		{}

	private:
		SimpleCacheDefaultDeltaExtension(SimpleCacheViewMode mode, std::unique_ptr<Hash256>&& pMerkleRoot)
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

		/// Prunes the cache at \a height.
		void prune(Height height) {
			// change the second byte
			(*m_pMerkleRoot)[1] = static_cast<uint8_t>(height.unwrap());
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
		BasicSimpleCacheViewExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: TViewExtension(mode, state)
				, m_mode(mode)
				, m_id(state.Id)
				, m_ids(m_id) {
			std::iota(m_ids.begin(), m_ids.end(), 1);
		}

	public:
		/// Gets the id.
		uint64_t id() const {
			return m_id;
		}

	public:
		/// Gets the size.
		size_t size() const {
			return static_cast<size_t>(m_id);
		}

		/// Returns \c true if \a id is contained by this view.
		bool contains(uint64_t id) const {
			return id <= m_id;
		}

	public:
		/// Finds the cache value identified by \a id.
		const_iterator find(uint64_t id) const {
			return const_iterator(id * id, contains(id));
		}

		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		uint64_t get(uint64_t id) const {
			return find(id).get();
		}

		/// Gets a value indicating whether or not an artifact with \a id is active at \a height.
		bool isActive(uint64_t id, Height height) const {
			return contains(id) && 0 == id % height.unwrap();
		}

	public:
		/// Gets a const iterator to the first cache element.
		auto begin() const {
			return m_ids.cbegin();
		}

		/// Gets a const iterator to the element following the last cache element.
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
		const uint64_t& m_id;
		std::vector<uint64_t> m_ids;
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
		BasicSimpleCacheDeltaExtension(SimpleCacheViewMode mode, const SimpleCacheState& state)
				: TDeltaExtension(mode, state)
				, m_id(state.Id)
		{}

	public:
		/// Gets the id.
		uint64_t id() const {
			return m_id;
		}

		/// Increments the id.
		void increment() {
			++m_id;
		}

	public:
		/// Finds the cache value identified by \a id.
		const_iterator find(uint64_t id) const {
			return const_iterator(id * id, contains(id));
		}

		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		uint64_t get(uint64_t id) const {
			return find(id).get();
		}

		/// Gets a value indicating whether or not an artifact with \a id is active at \a height.
		bool isActive(uint64_t id, Height height) const {
			return contains(id) && 0 == id % height.unwrap();
		}

	public:
		/// Gets the size.
		size_t size() const {
			return static_cast<size_t>(m_id);
		}

		/// Returns \c true if \a id is contained by this view.
		bool contains(uint64_t id) const {
			return id <= m_id;
		}

		/// Inserts \a id into the cache.
		void insert(uint64_t id) {
			m_id = id;
		}

	public:
		/// Sets the \a elements returned by delta element accessors (addedElements, modifiedElements, removedElements).
		void setElements(const std::array<uint64_t, 4>& elements) {
			m_elements = elements;
		}

	public:
		/// Gets the pointers to all added elements.
		std::unordered_set<const uint64_t*> addedElements() const {
			return { &m_elements[0] };
		}

		/// Gets the pointers to all modified elements.
		std::unordered_set<const uint64_t*> modifiedElements() const {
			return { &m_elements[2] };
		}

		/// Gets the pointers to all removed elements.
		std::unordered_set<const uint64_t*> removedElements() const {
			return { &m_elements[1], &m_elements[3] };
		}

	private:
		uint64_t m_id;
		std::array<uint64_t, 4> m_elements;
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
		using CacheValueType = uint64_t;
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
		/// Gets a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_mode, m_state);
		}

		/// Gets a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return CacheDeltaType(m_mode, m_state);
		}

		/// Gets a lockable cache delta based on this cache but without the ability
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
		uint64_t CacheId,
		typename TViewExtension = SimpleCacheDefaultViewExtension,
		typename TDeltaExtension = SimpleCacheDefaultDeltaExtension
	>
	class SimpleCacheT : public SimpleCacheExtension<TViewExtension, TDeltaExtension> {
	public:
		/// Unique cache identifier.
		static constexpr uint64_t Id = CacheId;

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
		static void Save(uint64_t value, io::OutputStream& output) {
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

		/// Purges \a value from \a cacheDelta.
		static void Purge(uint64_t value, DestinationType& cacheDelta) {
			CATAPULT_THROW_RUNTIME_ERROR_2("SimpleCacheExtensionStorage does not support Purge (value, id)", value, cacheDelta.id());
		}
	};

	using SimpleCacheStorageTraits = SimpleCacheExtensionStorageTraits<SimpleCacheDefaultViewExtension, SimpleCacheDefaultDeltaExtension>;

	// endregion
}}
