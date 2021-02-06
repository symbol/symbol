/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "Future.h"
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	/// Uses \a ioContext to process \a items in \a numPartitions batches and calls \a callback for each partition.
	/// Future is returned that is resolved when all items have been processed.
	template<typename TItems, typename TWorkCallback>
	thread::future<bool> ParallelForPartition(
			boost::asio::io_context& ioContext,
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
		auto numTotalItems = items.size();
		auto numRemainingItems = numTotalItems;
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
			auto startIndex = numTotalItems - numRemainingItems;
			auto batchIndex = numPartitions - numRemainingPartitions;
			boost::asio::post(ioContext, [callback, pParallelContext, itBegin, itEnd, startIndex, batchIndex]() {
				DecrementGuard threadOperationGuard(*pParallelContext);
				callback(itBegin, itEnd, startIndex, batchIndex);
			});

			numRemainingItems -= size;
			--numRemainingPartitions;
			itBegin = itEnd;
		}

		return pParallelContext->future();
	}

	/// Uses \a ioContext to process \a items in \a numPartitions batches and calls \a callback for each item.
	/// Future is returned that is resolved when all items have been processed.
	template<typename TItems, typename TWorkCallback>
	thread::future<bool> ParallelFor(boost::asio::io_context& ioContext, TItems& items, size_t numPartitions, TWorkCallback callback) {
		return ParallelForPartition(ioContext, items, numPartitions, [callback](auto itBegin, auto itEnd, auto startIndex, auto) {
			auto i = 0u;
			for (auto iter = itBegin; itEnd != iter; ++iter, ++i) {
				if (!callback(*iter, startIndex + i))
					break;
			}
		});
	}
}}
