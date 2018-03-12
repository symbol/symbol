#pragma once
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoDatabase.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/thread/FutureUtils.h"
#include <unordered_set>

namespace catapult { namespace mongo { namespace storages {

	namespace detail {
		template<typename TCacheTraits>
		class CacheStorageLoader {
		private:
			using CacheDeltaType = typename TCacheTraits::CacheDeltaType;

		private:
			enum class LoaderType { Unsorted, Sorted };
			using UnsortedLoaderFlag = std::integral_constant<LoaderType, LoaderType::Unsorted>;
			using SortedLoaderFlag = std::integral_constant<LoaderType, LoaderType::Sorted>;

		private:
			static constexpr size_t Checkpoint_Interval = 10'000;

		public:
			static void LoadAll(const MongoDatabase& database, CacheDeltaType& cache, const action& checkpoint) {
				auto collection = database[TCacheTraits::Collection_Name];

				mongocxx::options::find options;
				boost::optional<bsoncxx::document::value> ordering;
				if (PrepareOrdering(ordering, LoaderTypeAccessor<TCacheTraits>()))
					options.sort(ordering->view());

				auto counter = 0u;
				auto cursor = collection.find({}, options);
				for (const auto& document : cursor) {
					if (0 == ++counter % Checkpoint_Interval) {
						checkpoint();
						CATAPULT_LOG(info) << "committing " << counter << " documents to collection " << TCacheTraits::Collection_Name;
					}

					TCacheTraits::Insert(cache, document);
				}
			}

		private:
			template<typename T, typename = void>
			struct LoaderTypeAccessor
					: UnsortedLoaderFlag
			{};

			template<typename T>
			struct LoaderTypeAccessor<T, typename utils::traits::enable_if_type<decltype(T::LoadSortOrder())>::type>
					: SortedLoaderFlag
			{};

		private:
			static bool PrepareOrdering(boost::optional<bsoncxx::document::value>&, UnsortedLoaderFlag) {
				return false;
			}

			static bool PrepareOrdering(boost::optional<bsoncxx::document::value>& ordering, SortedLoaderFlag) {
				ordering = TCacheTraits::LoadSortOrder();
				return true;
			}
		};
	}

	/// Defines types for mongo cache storage given a cache descriptor.
	template<typename TDescriptor>
	struct BasicMongoCacheStorageTraits {
		/// The cache type.
		using CacheType = typename TDescriptor::CacheType;

		/// The cache delta type.
		using CacheDeltaType = typename TDescriptor::CacheDeltaType;

		/// The key type.
		using KeyType = typename TDescriptor::KeyType;

		/// The model type.
		using ModelType = typename TDescriptor::ValueType;

		/// Gets the key corresponding to a value.
		static constexpr auto GetId = TDescriptor::GetKeyFromValue;
	};

	/// A mongo cache storage that persists historical cache data using delete and insert.
	template<typename TCacheTraits>
	class MongoHistoricalCacheStorage : public ExternalCacheStorageT<typename TCacheTraits::CacheType> {
	private:
		using CacheDeltaType = typename TCacheTraits::CacheDeltaType;
		using ElementContainerType = typename TCacheTraits::ElementContainerType;
		using IdContainerType = typename TCacheTraits::IdContainerType;
		using LoadCheckpointFunc = typename ExternalCacheStorageT<typename TCacheTraits::CacheType>::LoadCheckpointFunc;

	public:
		/// Creates a cache storage around \a database, \a bulkWriter and \a networkIdentifier.
		MongoHistoricalCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter, model::NetworkIdentifier networkIdentifier)
				: m_database(std::move(database))
				, m_bulkWriter(bulkWriter)
				, m_networkIdentifier(networkIdentifier)
		{}

	private:
		void saveDelta(const CacheDeltaType& cache) override {
			auto addedElements = cache.addedElements();
			auto modifiedElements = cache.modifiedElements();
			auto removedElements = cache.removedElements();

			// 1. remove all modified and removed elements
			auto modifiedIds = GetIds(modifiedElements);
			auto removedIds = GetIds(removedElements);
			modifiedIds.insert(removedIds.cbegin(), removedIds.cend());
			removeAll(modifiedIds);

			// 2. insert new elements and modified elements
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

			auto numActualRemoved = mappers::ToUint32(deleteResult.get().deleted_count());
			if (!deleteResult || ids.size() > numActualRemoved) {
				std::ostringstream out;
				out
						<< "error deleting removed and modified " << TCacheTraits::Collection_Name << " elements"
						<< " (" << ids.size() << " expected, " << numActualRemoved << " actual)";
				CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
			}
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

			auto numActualInserted = mappers::ToUint32(aggregateResult.NumInserted);
			if (allModels.size() != numActualInserted) {
				std::ostringstream out;
				out
						<< "error inserting modified and added " << TCacheTraits::Collection_Name << " elements"
						<< " (" << allModels.size() << " expected, " << numActualInserted << " actual)";
				CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
			}
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
					<< std::string(TCacheTraits::Id_Property_Name)
					<< open_document
						<< "$in"
						<< open_array;

			for (auto id : ids)
				array << TCacheTraits::MapToMongoId(id);

			array << close_array;
			doc << close_document;
			return doc << finalize;
		}

	private:
		void loadAll(CacheDeltaType& cache, Height, const LoadCheckpointFunc& checkpoint) const override {
			detail::CacheStorageLoader<TCacheTraits>::LoadAll(m_database, cache, checkpoint);
		}

	private:
		MongoDatabase m_database;
		MongoBulkWriter& m_bulkWriter;
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// A mongo cache storage that persists flat cache data using delete and upsert.
	template<typename TCacheTraits>
	class MongoFlatCacheStorage : public ExternalCacheStorageT<typename TCacheTraits::CacheType> {
	private:
		using CacheDeltaType = typename TCacheTraits::CacheDeltaType;
		using KeyType = typename TCacheTraits::KeyType;
		using ModelType = typename TCacheTraits::ModelType;
		using ElementContainerType = std::unordered_set<const ModelType*>;
		using LoadCheckpointFunc = typename ExternalCacheStorageT<typename TCacheTraits::CacheType>::LoadCheckpointFunc;

	public:
		/// Creates a cache storage around \a database, \a bulkWriter and \a networkIdentifier.
		MongoFlatCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter, model::NetworkIdentifier networkIdentifier)
				: m_database(std::move(database))
				, m_bulkWriter(bulkWriter)
				, m_networkIdentifier(networkIdentifier)
		{}

	private:
		void saveDelta(const CacheDeltaType& cache) override {
			auto addedElements = cache.addedElements();
			auto modifiedElements = cache.modifiedElements();
			auto removedElements = cache.removedElements();

			// 1. remove all removed elements
			removeAll(removedElements);

			// 2. upsert new elements and modified elements
			modifiedElements.insert(addedElements.cbegin(), addedElements.cend());
			upsertAll(modifiedElements);
		}

	private:
		void removeAll(const ElementContainerType& elements) {
			if (elements.empty())
				return;

			auto deleteResults = m_bulkWriter.bulkDelete(TCacheTraits::Collection_Name, elements, CreateFilter).get();
			auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(deleteResults)));

			auto numActualRemoved = mappers::ToUint32(aggregateResult.NumDeleted);
			if (elements.size() != numActualRemoved) {
				std::ostringstream out;
				out
						<< "error deleting removed " << TCacheTraits::Collection_Name << " elements"
						<< " (" << elements.size() << " expected, " << numActualRemoved << " actual)";
				CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
			}
		}

		void upsertAll(const ElementContainerType& elements) {
			if (elements.empty())
				return;

			auto createDocument = [networkIdentifier = m_networkIdentifier](const auto* pModel, auto) {
				return TCacheTraits::MapToMongoDocument(*pModel, networkIdentifier);
			};
			auto upsertResults = m_bulkWriter.bulkUpsert(TCacheTraits::Collection_Name, elements, createDocument, CreateFilter).get();
			auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(upsertResults)));

			auto numActualUpserted = mappers::ToUint32(aggregateResult.NumModified) + mappers::ToUint32(aggregateResult.NumUpserted);
			if (elements.size() != numActualUpserted) {
				std::ostringstream out;
				out
						<< "error upserting modified and added " << TCacheTraits::Collection_Name << " elements"
						<< " (" << elements.size() << " expected, " << numActualUpserted << " actual)";
				CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
			}
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
		void loadAll(CacheDeltaType& cache, Height, const LoadCheckpointFunc& checkpoint) const override {
			detail::CacheStorageLoader<TCacheTraits>::LoadAll(m_database, cache, checkpoint);
		}

	private:
		MongoDatabase m_database;
		MongoBulkWriter& m_bulkWriter;
		model::NetworkIdentifier m_networkIdentifier;
	};
}}}
