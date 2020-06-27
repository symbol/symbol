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
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoStorageContext.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/thread/FutureUtils.h"
#include <set>
#include <unordered_set>

namespace catapult { namespace mongo { namespace storages {

	namespace detail {
		/// Defines a mongo element filter.
		template<typename TCacheTraits, typename TElementContainerType>
		struct MongoElementFilter {
		public:
			/// Filters all elements common to added elements (\a addedElements) and removed elements (\a removedElements).
			static void RemoveCommonElements(TElementContainerType& addedElements, TElementContainerType& removedElements) {
				auto commonIds = GetCommonIds(addedElements, removedElements);
				RemoveElements(addedElements, commonIds);
				RemoveElements(removedElements, commonIds);
			}

		private:
			static auto GetCommonIds(const TElementContainerType& addedElements, const TElementContainerType& removedElements) {
				std::set<typename TCacheTraits::KeyType> addedElementIds;
				for (const auto* pAddedElement : addedElements)
					addedElementIds.insert(TCacheTraits::GetId(*pAddedElement));

				std::set<typename TCacheTraits::KeyType> removedElementIds;
				for (const auto* pRemovedElement : removedElements)
					removedElementIds.insert(TCacheTraits::GetId(*pRemovedElement));

				std::set<typename TCacheTraits::KeyType> commonIds;
				std::set_intersection(
						addedElementIds.cbegin(),
						addedElementIds.cend(),
						removedElementIds.cbegin(),
						removedElementIds.cend(),
						std::inserter(commonIds, commonIds.cbegin()));
				return commonIds;
			}

			static void RemoveElements(TElementContainerType& elements, std::set<typename TCacheTraits::KeyType>& ids) {
				for (auto iter = elements.cbegin(); elements.cend() != iter;) {
					if (ids.cend() != ids.find(TCacheTraits::GetId(**iter)))
						iter = elements.erase(iter);
					else
						++iter;
				}
			}
		};
	}

	/// Defines types for mongo cache storage given a cache descriptor.
	template<typename TDescriptor>
	struct BasicMongoCacheStorageTraits {
		/// Cache type.
		using CacheType = typename TDescriptor::CacheType;

		/// Cache delta type.
		using CacheDeltaType = typename TDescriptor::CacheDeltaType;

		/// Key type.
		using KeyType = typename TDescriptor::KeyType;

		/// Model type.
		using ModelType = typename TDescriptor::ValueType;

		/// Gets the key corresponding to a value.
		static constexpr auto GetId = TDescriptor::GetKeyFromValue;
	};

	/// Mongo cache storage that persists historical cache data using delete and insert.
	template<typename TCacheTraits>
	class MongoHistoricalCacheStorage : public ExternalCacheStorageT<typename TCacheTraits::CacheType> {
	private:
		using CacheChangesType = cache::SingleCacheChangesT<
			typename TCacheTraits::CacheDeltaType,
			typename TCacheTraits::CacheType::CacheValueType>;
		using ElementContainerType = typename TCacheTraits::ElementContainerType;
		using IdContainerType = typename TCacheTraits::IdContainerType;

	public:
		/// Creates a cache storage around \a storageContext and \a networkIdentifier.
		MongoHistoricalCacheStorage(MongoStorageContext& storageContext, model::NetworkIdentifier networkIdentifier)
				: m_database(storageContext.createDatabaseConnection())
				, m_errorPolicy(storageContext.createCollectionErrorPolicy(TCacheTraits::Collection_Name))
				, m_bulkWriter(storageContext.bulkWriter())
				, m_networkIdentifier(networkIdentifier)
		{}

	private:
		void saveDelta(const CacheChangesType& changes) override {
			auto addedElements = changes.addedElements();
			auto modifiedElements = changes.modifiedElements();
			auto removedElements = changes.removedElements();

			// 1. remove elements common to both added and removed
			detail::MongoElementFilter<TCacheTraits, ElementContainerType>::RemoveCommonElements(addedElements, removedElements);

			// 2. remove all modified and removed elements from db
			auto modifiedIds = GetIds(modifiedElements);
			auto removedIds = GetIds(removedElements);
			modifiedIds.insert(removedIds.cbegin(), removedIds.cend());
			removeAll(modifiedIds);

			// 3. insert new elements and modified elements into db
			modifiedElements.insert(addedElements.cbegin(), addedElements.cend());
			insertAll(modifiedElements);
		}

	private:
		void removeAll(const IdContainerType& ids) {
			if (ids.empty())
				return;

			auto collection = m_database[TCacheTraits::Collection_Name];

			auto filter = CreateDeleteFilter(ids);
			auto deleteResult = collection.delete_many(filter.view());
			m_errorPolicy.checkDeletedAtLeast(ids.size(), BulkWriteResult(deleteResult.value().result()), "removed and modified elements");
		}

		void insertAll(const ElementContainerType& elements) {
			if (elements.empty())
				return;

			std::vector<typename TCacheTraits::ModelType> allModels;
			for (const auto* pElement : elements) {
				auto models = TCacheTraits::MapToMongoModels(*pElement, m_networkIdentifier);
				std::move(models.begin(), models.end(), std::back_inserter(allModels));
			}

			auto insertResults = m_bulkWriter.bulkInsert(TCacheTraits::Collection_Name, allModels, [](const auto& model, auto) {
				return TCacheTraits::MapToMongoDocument(model);
			}).get();

			auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(insertResults)));
			m_errorPolicy.checkInserted(allModels.size(), aggregateResult, "modified and added elements");
		}

	private:
		static IdContainerType GetIds(const ElementContainerType& elements) {
			IdContainerType ids;
			for (const auto* pElement : elements)
				ids.insert(TCacheTraits::GetId(*pElement));

			return ids;
		}

		static bsoncxx::document::value CreateDeleteFilter(const IdContainerType& ids) {
			using namespace bsoncxx::builder::stream;

			if (ids.empty())
				return document() << finalize;

			document doc;
			auto array = doc
					<< std::string(TCacheTraits::Id_Property_Name) << open_document
						<< "$in"
						<< open_array;

			for (auto id : ids)
				array << TCacheTraits::MapToMongoId(id);

			array << close_array;
			doc << close_document;
			return doc << finalize;
		}

	private:
		MongoDatabase m_database;
		MongoErrorPolicy m_errorPolicy;
		MongoBulkWriter& m_bulkWriter;
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// Mongo cache storage that persists flat cache data using delete and upsert.
	template<typename TCacheTraits>
	class MongoFlatCacheStorage : public ExternalCacheStorageT<typename TCacheTraits::CacheType> {
	private:
		using CacheChangesType = cache::SingleCacheChangesT<typename TCacheTraits::CacheDeltaType, typename TCacheTraits::ModelType>;
		using KeyType = typename TCacheTraits::KeyType;
		using ModelType = typename TCacheTraits::ModelType;
		using ElementContainerType = std::unordered_set<const ModelType*>;

	public:
		/// Creates a cache storage around \a storageContext and \a networkIdentifier.
		MongoFlatCacheStorage(MongoStorageContext& storageContext, model::NetworkIdentifier networkIdentifier)
				: m_database(storageContext.createDatabaseConnection())
				, m_errorPolicy(storageContext.createCollectionErrorPolicy(TCacheTraits::Collection_Name))
				, m_bulkWriter(storageContext.bulkWriter())
				, m_networkIdentifier(networkIdentifier)
		{}

	private:
		void saveDelta(const CacheChangesType& changes) override {
			auto addedElements = changes.addedElements();
			auto modifiedElements = changes.modifiedElements();
			auto removedElements = changes.removedElements();

			// 1. remove elements common to both added and removed
			detail::MongoElementFilter<TCacheTraits, ElementContainerType>::RemoveCommonElements(addedElements, removedElements);

			// 2. remove all removed elements from db
			removeAll(removedElements);

			// 3. upsert new elements and modified elements into db
			modifiedElements.insert(addedElements.cbegin(), addedElements.cend());
			upsertAll(modifiedElements);
		}

	private:
		void removeAll(const ElementContainerType& elements) {
			if (elements.empty())
				return;

			auto deleteResults = m_bulkWriter.bulkDelete(TCacheTraits::Collection_Name, elements, CreateFilter).get();
			auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(deleteResults)));
			m_errorPolicy.checkDeleted(elements.size(), aggregateResult, "removed elements");
		}

		void upsertAll(const ElementContainerType& elements) {
			if (elements.empty())
				return;

			auto createDocument = [networkIdentifier = m_networkIdentifier](const auto* pModel, auto) {
				return TCacheTraits::MapToMongoDocument(*pModel, networkIdentifier);
			};
			auto upsertResults = m_bulkWriter.bulkUpsert(TCacheTraits::Collection_Name, elements, createDocument, CreateFilter).get();
			auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(upsertResults)));
			m_errorPolicy.checkUpserted(elements.size(), aggregateResult, "modified and added elements");
		}

	private:
		static bsoncxx::document::value CreateFilter(const ModelType* pModel) {
			return CreateFilterByKey(TCacheTraits::GetId(*pModel));
		}

		static bsoncxx::document::value CreateFilterByKey(const KeyType& key) {
			using namespace bsoncxx::builder::stream;

			return document() << std::string(TCacheTraits::Id_Property_Name) << TCacheTraits::MapToMongoId(key) << finalize;
		}

	private:
		MongoDatabase m_database;
		MongoErrorPolicy m_errorPolicy;
		MongoBulkWriter& m_bulkWriter;
		model::NetworkIdentifier m_networkIdentifier;
	};
}}}
