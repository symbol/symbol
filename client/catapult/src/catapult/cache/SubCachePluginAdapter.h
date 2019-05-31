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
#include "CacheChangesStorageAdapter.h"
#include "CacheStorageAdapter.h"
#include "SubCachePlugin.h"
#include <memory>
#include <sstream>

namespace catapult { namespace cache {

	/// A SubCachePlugin implementation that wraps a SynchronizedCache.
	template<typename TCache, typename TStorageTraits>
	class SubCachePluginAdapter : public SubCachePlugin {
	public:
		/// Creates an adapter around \a pCache.
		explicit SubCachePluginAdapter(std::unique_ptr<TCache>&& pCache) : m_pCache(std::move(pCache)) {
			std::ostringstream out;
			out << TCache::Name << " (id = " << TCache::Id << ")";
			m_name = out.str();
		}

	public:
		const std::string& name() const override {
			return m_name;
		}

		size_t id() const override {
			return TCache::Id;
		}

	public:
		std::unique_ptr<const SubCacheView> createView() const override {
			return std::make_unique<const SubCacheViewAdapter<decltype(m_pCache->createView())>>(
					m_pCache->createView(),
					makeSubCacheViewIdentifier(SubCacheViewType::View));
		}

		std::unique_ptr<SubCacheView> createDelta() override {
			return std::make_unique<SubCacheViewAdapter<decltype(m_pCache->createDelta())>>(
					m_pCache->createDelta(),
					makeSubCacheViewIdentifier(SubCacheViewType::Delta));
		}

		std::unique_ptr<DetachedSubCacheView> createDetachedDelta() const override {
			return std::make_unique<DetachedSubCacheViewAdapter<decltype(m_pCache->createDetachedDelta())>>(
					m_pCache->createDetachedDelta(),
					makeSubCacheViewIdentifier(SubCacheViewType::DetachedDelta));
		}

		void commit() override {
			m_pCache->commit();
		}

	public:
		const void* get() const override {
			return m_pCache.get();
		}

	public:
		std::unique_ptr<CacheStorage> createStorage() override {
			return IsCacheStorageSupported(*m_pCache)
					? std::make_unique<CacheStorageAdapter<TCache, TStorageTraits>>(*m_pCache)
					: nullptr;
		}

		std::unique_ptr<CacheChangesStorage> createChangesStorage() const override {
			return std::make_unique<CacheChangesStorageAdapter<TCache, TStorageTraits>>(*m_pCache);
		}

	public:
		/// Gets a typed reference to the underlying cache.
		TCache& cache() {
			return *m_pCache;
		}

	private:
		auto makeSubCacheViewIdentifier(SubCacheViewType viewType) const {
			SubCacheViewIdentifier viewIdentifier{};
			std::memcpy(viewIdentifier.CacheName.data(), name().data(), viewIdentifier.CacheName.size());
			viewIdentifier.CacheId = id();
			viewIdentifier.ViewType = viewType;
			return viewIdentifier;
		}

	private:
		static bool IsCacheStorageSupported(const TCache& cache) {
			return !!cache.createView()->tryMakeIterableView();
		}

	private:
		// region SubCacheViewAdapter

		template<typename TView>
		class SubCacheViewAdapter : public SubCacheView {
		public:
			explicit SubCacheViewAdapter(TView&& view, const SubCacheViewIdentifier& id)
					: m_view(std::move(view))
					, m_id(id)
			{}

		private:
			auto merkleRootAccessor() const {
				// need to dereference to get underlying view type from LockedCacheView
				using UnderlyingViewType = std::remove_reference_t<decltype(*m_view)>;
				return MerkleRootAccessor<UnderlyingViewType>();
			}

			auto merkleRootMutator() {
				// need to dereference to get underlying view type from LockedCacheView
				using UnderlyingViewType = std::remove_reference_t<decltype(*m_view)>;
				return MerkleRootMutator<UnderlyingViewType>();
			}

		public:
			const SubCacheViewIdentifier& id() const override {
				return m_id;
			}

			const void* get() const override {
				return &*m_view;
			}

			void* get() override {
				return &*m_view;
			}

			bool supportsMerkleRoot() const override {
				return SupportsMerkleRoot(m_view, merkleRootAccessor());
			}

			bool tryGetMerkleRoot(Hash256& merkleRoot) const override {
				return TryGetMerkleRoot(m_view, merkleRoot, merkleRootAccessor());
			}

			bool trySetMerkleRoot(const Hash256& merkleRoot) override {
				return TrySetMerkleRoot(m_view, merkleRoot, merkleRootMutator());
			}

			void updateMerkleRoot(Height height) override {
				UpdateMerkleRoot(m_view, height, merkleRootMutator());
			}

			const void* asReadOnly() const override {
				return &m_view->asReadOnly();
			}

		private:
			enum class MerkleRootType { Unsupported, Supported };
			using UnsupportedMerkleRootFlag = std::integral_constant<MerkleRootType, MerkleRootType::Unsupported>;
			using SupportedMerkleRootFlag = std::integral_constant<MerkleRootType, MerkleRootType::Supported>;

			template<typename T, typename = void>
			struct MerkleRootAccessor : public UnsupportedMerkleRootFlag {};

			template<typename T>
			struct MerkleRootAccessor<
					T,
					utils::traits::is_type_expression_t<decltype(reinterpret_cast<const T*>(0)->tryGetMerkleRoot())>>
					: public SupportedMerkleRootFlag
			{};

			template<typename T, typename = void>
			struct MerkleRootMutator : public UnsupportedMerkleRootFlag {};

			template<typename T>
			struct MerkleRootMutator<
					T,
					utils::traits::is_type_expression_t<decltype(reinterpret_cast<T*>(0)->setMerkleRoot(Hash256()))>>
					: public SupportedMerkleRootFlag
			{};

			static bool SupportsMerkleRoot(const TView&, UnsupportedMerkleRootFlag) {
				return false;
			}

			static bool SupportsMerkleRoot(const TView& view, SupportedMerkleRootFlag) {
				return view->supportsMerkleRoot();
			}

			static bool TryGetMerkleRoot(const TView&, Hash256&, UnsupportedMerkleRootFlag) {
				return false;
			}

			static bool TryGetMerkleRoot(const TView& view, Hash256& merkleRoot, SupportedMerkleRootFlag) {
				auto result = view->tryGetMerkleRoot();
				merkleRoot = result.first;
				return result.second;
			}

			static bool TrySetMerkleRoot(TView&, const Hash256&, UnsupportedMerkleRootFlag) {
				return false;
			}

			static bool TrySetMerkleRoot(TView& view, const Hash256& merkleRoot, SupportedMerkleRootFlag) {
				if (!view->supportsMerkleRoot())
					return false;

				view->setMerkleRoot(merkleRoot);
				return true;
			}

			static void UpdateMerkleRoot(TView&, Height, UnsupportedMerkleRootFlag)
			{}

			static void UpdateMerkleRoot(TView& view, Height height, SupportedMerkleRootFlag) {
				view->updateMerkleRoot(height);
			}

		private:
			TView m_view;
			SubCacheViewIdentifier m_id;
		};

		// endregion

		// region DetachedSubCacheViewAdapter

		template<typename TLockableCacheDelta>
		class DetachedSubCacheViewAdapter : public DetachedSubCacheView {
		public:
			explicit DetachedSubCacheViewAdapter(TLockableCacheDelta&& lockableCacheDelta, const SubCacheViewIdentifier& id)
					: m_lockableCacheDelta(std::move(lockableCacheDelta))
					, m_id(id)
			{}

		public:
			std::unique_ptr<SubCacheView> tryLock() {
				auto delta = m_lockableCacheDelta.tryLock();
				return delta ? std::make_unique<SubCacheViewAdapter<decltype(delta)>>(std::move(delta), m_id) : nullptr;
			}

		private:
			TLockableCacheDelta m_lockableCacheDelta;
			SubCacheViewIdentifier m_id;
		};

		// endregion

	private:
		std::unique_ptr<TCache> m_pCache;
		std::string m_name;
	};
}}
