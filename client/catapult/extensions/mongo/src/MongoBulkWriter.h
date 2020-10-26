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
#include "BulkWriteResult.h"
#include "catapult/model/Elements.h"
#include "catapult/state/AccountState.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/thread/ParallelFor.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <boost/asio/io_context.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/config/version.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/pool.hpp>
#include <unordered_set>

namespace catapult { namespace mongo {

	/// Class for writing bulk data to the mongo database.
	/// \note The bulk writer supports inserting, upserting and deleting documents.
	class MongoBulkWriter final : public std::enable_shared_from_this<MongoBulkWriter> {
	private:
		struct BulkWriteParams {
		public:
			BulkWriteParams(MongoBulkWriter& mongoBulkWriter, const std::string& collectionName)
					: pConnection(mongoBulkWriter.m_connectionPool.acquire())
					, Database(pConnection->database(mongoBulkWriter.m_dbName))
					, Collection(Database[collectionName])
					, Bulk(Collection.create_bulk_write())
		{}

		public:
			mongocxx::pool::entry pConnection;
			mongocxx::database Database;
			mongocxx::collection Collection;
			mongocxx::bulk_write Bulk;
		};

		using AccountStates = std::unordered_set<std::shared_ptr<const state::AccountState>>;
		using BulkWriteResultFuture = thread::future<std::vector<thread::future<BulkWriteResult>>>;

		template<typename TEntity>
		using AppendOperation = consumer<mongocxx::bulk_write&, const TEntity&, uint32_t>;

		template<typename TEntity>
		using CreateDocument = std::function<bsoncxx::document::value (const TEntity&, uint32_t)>;

		template<typename TEntity>
		using CreateDocuments = std::function<std::vector<bsoncxx::document::value> (const TEntity&, uint32_t)>;

		template<typename TEntity>
		using CreateFilter = std::function<bsoncxx::document::value (const TEntity&)>;

	private:
		MongoBulkWriter(const mongocxx::uri& uri, const std::string& dbName, thread::IoThreadPool& pool)
				: m_dbName(dbName)
				, m_pool(pool)
				, m_connectionPool(uri)
		{}

	public:
		/// Creates a mongo bulk writer connected to \a uri that will use database \a dbName for bulk writes.
		/// \note Concurrent writes are performed using the specified thread \a pool.
		static std::shared_ptr<MongoBulkWriter> Create(const mongocxx::uri& uri, const std::string& dbName, thread::IoThreadPool& pool) {
			// cannot use make_shared with private constructor
			auto pBackingMemory = utils::MakeUniqueWithSize<uint8_t>(sizeof(MongoBulkWriter));
			auto pWriterRaw = new (pBackingMemory.get()) MongoBulkWriter(uri, dbName, pool);
			auto pWriter = std::shared_ptr<MongoBulkWriter>(pWriterRaw);
			pBackingMemory.release();
			return pWriter;
		}

	public:
		/// Inserts \a entities into the collection named \a collectionName using a one-to-one mapping of entities
		/// to documents (\a createDocument).
		template<typename TContainer>
		BulkWriteResultFuture bulkInsert(
				const std::string& collectionName,
				const TContainer& entities,
				const CreateDocument<typename TContainer::value_type>& createDocument) {
			auto appendOperation = [createDocument](auto& bulk, const auto& entity, auto index) {
				auto entityDocument = createDocument(entity, index);
				bulk.append(mongocxx::model::insert_one(entityDocument.view()));
			};

			return bulkWrite<TContainer>(collectionName, entities, appendOperation);
		}

		/// Inserts \a entities into the collection named \a collectionName using a one-to-many mapping of entities
		/// to documents (\a createDocuments).
		template<typename TContainer>
		BulkWriteResultFuture bulkInsert(
				const std::string& collectionName,
				const TContainer& entities,
				const CreateDocuments<typename TContainer::value_type>& createDocuments) {
			auto appendOperation = [createDocuments](auto& bulk, const auto& entity, auto index) {
				for (const auto& entityDocument : createDocuments(entity, index))
					bulk.append(mongocxx::model::insert_one(entityDocument.view()));
			};

			return bulkWrite<TContainer>(collectionName, entities, appendOperation);
		}

		/// Upserts \a entities into the collection named \a collectionName using a one-to-one mapping of entities
		/// to documents (\a createDocument) matching the specified entity filter (\a createFilter).
		template<typename TContainer>
		BulkWriteResultFuture bulkUpsert(
				const std::string& collectionName,
				const TContainer& entities,
				const CreateDocument<typename TContainer::value_type>& createDocument,
				const CreateFilter<typename TContainer::value_type>& createFilter) {
			auto appendOperation = [createDocument, createFilter](auto& bulk, const auto& entity, auto index) {
				auto entityDocument = createDocument(entity, index);
				auto filter = createFilter(entity);
				mongocxx::model::replace_one replace_op(filter.view(), entityDocument.view());
				replace_op.upsert(true);
				bulk.append(replace_op);
			};

			return bulkWrite<TContainer>(collectionName, entities, appendOperation);
		}

		/// Deletes \a entities from the collection named \a collectionName matching the specified entity filter (\a createFilter).
		template<typename TContainer>
		BulkWriteResultFuture bulkDelete(
				const std::string& collectionName,
				const TContainer& entities,
				const CreateFilter<typename TContainer::value_type>& createFilter) {
			auto appendOperation = [createFilter](auto& bulk, const auto& entity, auto) {
				auto filter = createFilter(entity);
				bulk.append(mongocxx::model::delete_many(filter.view()));
			};

			return bulkWrite<TContainer>(collectionName, entities, appendOperation);
		}

	private:
		thread::future<BulkWriteResult> handleBulkOperation(
				const std::string& collectionName,
				std::shared_ptr<BulkWriteParams>&& pBulkWriteParams) {
			// note: pBulkWriteParams depends on pThis (pBulkWriteParams.pConnection depends on pThis.m_connectionPool)
			// it's crucial to move pBulkWriteParams into lambda, otherwise it would be copied while pThis would be moved
			auto pPromise = std::make_shared<thread::promise<BulkWriteResult>>();
			auto handler = [pThis = shared_from_this(), collectionName, pBulkWriteParams{std::move(pBulkWriteParams)}, pPromise]() {
				pThis->bulkWrite(collectionName, *pBulkWriteParams, *pPromise);
			};

			boost::asio::post(m_pool.ioContext(), handler);
			return pPromise->get_future();
		}

		void bulkWrite(const std::string& collectionName, BulkWriteParams& bulkWriteParams, thread::promise<BulkWriteResult>& promise) {
			try {
				// if something goes wrong mongo will throw, else a result is always available
				auto result = bulkWriteParams.Bulk.execute().value();
				promise.set_value(BulkWriteResult(result));
			} catch (const mongocxx::bulk_write_exception& ex) {
				std::ostringstream out;
				out
						<< "bulk '" << collectionName << "' operation failed"
						<< std::endl << "code: " << ex.code().message()
						<< std::endl << "what: " << ex.what();

				if (ex.raw_server_error()) {
					auto description = bsoncxx::to_json(ex.raw_server_error().value());
					out << std::endl << description;
				} else {
					out << std::endl << "no server error";
				}

				CATAPULT_LOG(fatal) << out.str().c_str();
				promise.set_exception(std::make_exception_ptr(catapult_runtime_error(out.str().c_str())));
			}
		}

		struct BulkWriteContext {
		public:
			explicit BulkWriteContext(size_t numOperations) : m_futures(numOperations)
			{}

		public:
			BulkWriteResultFuture aggregateFuture() {
				return thread::when_all(std::move(m_futures));
			}

			void setFutureAt(size_t index, thread::future<BulkWriteResult>&& future) {
				m_futures[index] = std::move(future);
			}

		private:
			std::vector<thread::future<BulkWriteResult>> m_futures;
		};

		template<typename TEntity, typename TContainer>
		BulkWriteResultFuture bulkWrite(
				const std::string& collectionName,
				const TContainer& entities,
				const AppendOperation<typename TContainer::value_type>& appendOperation) {
			if (entities.empty())
				return thread::make_ready_future(std::vector<thread::future<BulkWriteResult>>());

			auto numThreads = m_pool.numWorkerThreads();
			auto pContext = std::make_shared<BulkWriteContext>(std::min<size_t>(entities.size(), numThreads));
			auto workCallback = [pThis = shared_from_this(), entitiesStart = entities.cbegin(), collectionName, appendOperation, pContext](
					auto itBegin,
					auto itEnd,
					auto startIndex,
					auto batchIndex) {
				auto pBulkWriteParams = std::make_shared<BulkWriteParams>(*pThis, collectionName);

				auto index = static_cast<uint32_t>(startIndex);
				for (auto iter = itBegin; itEnd != iter; ++iter, ++index)
					appendOperation(pBulkWriteParams->Bulk, *iter, index);

				pContext->setFutureAt(batchIndex, pThis->handleBulkOperation(collectionName, std::move(pBulkWriteParams)));
			};

			auto& ioContext = m_pool.ioContext();
			return thread::compose(thread::ParallelForPartition(ioContext, entities, numThreads, workCallback), [pContext](const auto&) {
				return pContext->aggregateFuture();
			});
		}

	private:
		std::string m_dbName;
		thread::IoThreadPool& m_pool;
		mongocxx::pool m_connectionPool;
	};
}}
