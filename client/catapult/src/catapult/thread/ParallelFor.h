#pragma once
#include "Future.h"
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	/// Uses \a service to process \a items in \a numPartitions batches and calls \a callback for each partition.
	/// A future is returned that is resolved when all items have been processed.
	template<typename TItems, typename TWorkCallback>
	thread::future<bool> ParallelForPartition(
			boost::asio::io_service& service,
			TItems& items,
			size_t numPartitions,
			TWorkCallback callback) {
		// region ParallelContext
		class ParallelContext {
		public:
			ParallelContext() : m_numOutstandingOperations(1) // note that the work partitioning is the initial operation
			{}

		public:
			auto future() {
				return m_promise.get_future();
			}

		public:
			void incrementOutstandingOperations() {
				++m_numOutstandingOperations;
			}

			void decrementOutstandingOperations() {
				if (0 != --m_numOutstandingOperations)
					return;

				m_promise.set_value(true);
			}

		private:
			std::atomic<size_t> m_numOutstandingOperations;
			thread::promise<bool> m_promise;
		};

		// endregion

		// region DecrementGuard

		class DecrementGuard {
		public:
			explicit DecrementGuard(ParallelContext& context) : m_context(context)
			{}

			~DecrementGuard() {
				m_context.decrementOutstandingOperations();
			}

		private:
			ParallelContext& m_context;
		};

		// endregion

		auto pParallelContext = std::make_shared<ParallelContext>();
		DecrementGuard mainOperationGuard(*pParallelContext);

		auto numRemainingPartitions = numPartitions;
		auto numRemainingItems = items.size();
		auto itBegin = items.begin();
		while (numRemainingItems > 0) {
			// note: in the case that numRemainingItems is not divisible by numRemainingPartitions,
			//       give the current partition one more item in order to ensure that
			//       the partitions cover all items
			auto isDivisible = 0 == numRemainingItems % numRemainingPartitions;
			auto size = numRemainingItems / numRemainingPartitions + (isDivisible ? 0 : 1);
			auto itEnd = itBegin;
			std::advance(itEnd, static_cast<typename decltype(itEnd)::difference_type>(size));

			// each thread captures pParallelContext by value, which keeps that object alive
			pParallelContext->incrementOutstandingOperations();
			auto batchIndex = numPartitions - numRemainingPartitions;
			service.post([callback, pParallelContext, itBegin, itEnd, batchIndex]() {
				DecrementGuard threadOperationGuard(*pParallelContext);
				callback(itBegin, itEnd, batchIndex);
			});

			numRemainingItems -= size;
			--numRemainingPartitions;
			itBegin = itEnd;
		}

		return pParallelContext->future();
	}

	/// Uses \a service to process \a items in \a numPartitions batches and calls \a callback for each item.
	/// A future is returned that is resolved when all items have been processed.
	template<typename TItems, typename TWorkCallback>
	thread::future<bool> ParallelFor(
			boost::asio::io_service& service,
			TItems& items,
			size_t numPartitions,
			TWorkCallback callback) {
		return ParallelForPartition(service, items, numPartitions, [callback](auto itBegin, auto itEnd, auto) {
			std::all_of(itBegin, itEnd, [callback](auto& item) {
				return callback(item);
			});
		});
	}
}}
