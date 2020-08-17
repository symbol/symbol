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
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/TransactionInfoSerializer.h"
#include "tests/test/core/NonOwningOutputStream.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"

namespace catapult { namespace test {

	/// File transactions change subscriber storage context.
	template<typename SubscriberTraits>
	class FileTransactionsChangeStorageContext {
	public:
		using TransactionInfosReference = std::reference_wrapper<const model::TransactionInfosSet>;
		using SubscriberType = typename SubscriberTraits::SubscriberType;
		using OperationType = typename SubscriberTraits::OperationType;

	public:
		/// Creates test context.
		FileTransactionsChangeStorageContext()
				: m_mockStream(m_buffer)
				, m_pSubscriber(SubscriberTraits::Create(std::make_unique<test::NonOwningOutputStream>(m_mockStream)))
		{}

	public:
		/// Gets the subscriber.
		SubscriberType& subscriber() {
			return *m_pSubscriber;
		}

		/// Asserts nothing was written to storage.
		void assertEmptyBuffer() {
			EXPECT_TRUE(m_buffer.empty());
		}

		/// Asserts content of file storage matches \a expected transaction infos.
		void assertFileContents(const std::vector<std::pair<OperationType, TransactionInfosReference>>& expected) {
			auto inputStream = createInputStream();

			for (const auto& operationInfosPair : expected) {
				auto operationType = static_cast<OperationType>(io::Read8(inputStream));
				EXPECT_EQ(operationInfosPair.first, operationType);
				assertTransactionInfos(inputStream, operationInfosPair.second.get());
			}

			EXPECT_EQ(m_buffer.size(), inputStream.position());
		}

		/// Asserts there have been \a numFlushes flushes.
		void assertNumFlushes(size_t numFlushes) {
			EXPECT_EQ(numFlushes, m_mockStream.numFlushes());
		}

	protected:
		/// Creates input stream around internal storage.
		auto createInputStream() {
			return io::BufferInputStreamAdapter<std::vector<uint8_t>>(m_buffer);
		}

	private:
		void assertTransactionInfos(io::InputStream& inputStream, const model::TransactionInfosSet& expectedTransactionInfos) {
			// Arrange:
			model::TransactionInfosSet transactionInfos;
			io::ReadTransactionInfos(inputStream, transactionInfos);

			// Assert:
			test::AssertEquivalent(expectedTransactionInfos, transactionInfos);
		}

	private:
		std::vector<uint8_t> m_buffer;
		mocks::MockMemoryStream m_mockStream;
		std::unique_ptr<SubscriberType> m_pSubscriber;
	};
}}
