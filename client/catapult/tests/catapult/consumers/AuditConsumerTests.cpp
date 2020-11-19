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

#include "catapult/consumers/AuditConsumer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

using catapult::disruptor::ConsumerInput;
using catapult::disruptor::InputSource;

namespace catapult { namespace consumers {

#define TEST_CLASS AuditConsumerTests

	namespace {
		template<typename TAction>
		void RunAuditConsumerTest(TAction action) {
			// Arrange:
			test::TempDirectoryGuard tempDirectoryGuard;
			auto consumer = CreateAuditConsumer(tempDirectoryGuard.name());

			// Act:
			action(consumer, tempDirectoryGuard.name());
		}

		class FileContentsChecker {
		public:
			explicit FileContentsChecker(const std::string& filename)
					: m_filename(filename)
					, m_file(m_filename, io::OpenMode::Read_Only, io::LockMode::None)
			{}

		public:
			void checkHeader(InputSource expectedSource, const Key& expectedSourcePublicKey) {
				auto source = static_cast<InputSource>(io::Read32(m_file));
				Key sourcePublicKey;
				m_file.read(sourcePublicKey);

				EXPECT_EQ(expectedSource, source) << m_filename;
				EXPECT_EQ(expectedSourcePublicKey, sourcePublicKey) << m_filename;
			}

			void checkEntry(const model::VerifiableEntity& expectedEntity) {
				std::vector<uint8_t> entityBuffer(expectedEntity.Size);
				m_file.read(entityBuffer);

				EXPECT_EQ(reinterpret_cast<const model::VerifiableEntity&>(entityBuffer[0]), expectedEntity) << m_filename;
			}

			void checkEof() {
				EXPECT_EQ(m_file.size(), m_file.position()) << m_filename;
			}

		private:
			std::string m_filename;
			io::RawFile m_file;
		};

		template<typename TRange>
		auto MakeConsumerInput(TRange&& range, const Key& identityKey, InputSource inputSource) {
			using AnnotatedEntityRange = model::AnnotatedEntityRange<typename TRange::value_type>;
			return ConsumerInput(AnnotatedEntityRange(std::move(range), { identityKey, "fake-host-from-audit-consumer" }), inputSource);
		}

		void AssertFileContents(
				const std::filesystem::path& filename,
				InputSource expectedSource,
				const Key& expectedSourcePublicKey,
				const model::VerifiableEntity& expectedEntity) {
			ASSERT_TRUE(std::filesystem::exists(filename));

			FileContentsChecker checker(filename.generic_string());
			checker.checkHeader(expectedSource, expectedSourcePublicKey);
			checker.checkEntry(expectedEntity);
			checker.checkEof();
		}

		template<typename TRangeFactory>
		void AssertCanProcessInputWithEntities(TRangeFactory rangeFactory, uint32_t numEntities) {
			// Arrange:
			RunAuditConsumerTest([rangeFactory, numEntities](const auto& consumer, const auto& auditDirectory) {
				auto range = rangeFactory(numEntities);
				auto rangeCopy = decltype(range)::CopyRange(range);
				auto key = test::GenerateRandomByteArray<Key>();

				// Act:
				auto result = consumer(MakeConsumerInput(std::move(range), key, InputSource::Remote_Pull));

				// Assert:
				test::AssertContinued(result);

				auto filename = std::filesystem::path(auditDirectory) / "1";
				ASSERT_TRUE(std::filesystem::exists(filename));

				auto iter = rangeCopy.cbegin();
				FileContentsChecker checker(filename.generic_string());
				checker.checkHeader(InputSource::Remote_Pull, key);
				for (auto i = 0u; i < numEntities; ++i)
					checker.checkEntry(*iter++);

				checker.checkEof();
			});
		}
	}

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		// Arrange:
		RunAuditConsumerTest([](const auto& consumer, const auto&) {
			// Assert:
			test::AssertPassthroughForEmptyInput(consumer);
		});
	}

	TEST(TEST_CLASS, CanProcessInputWithSingleTransaction) {
		AssertCanProcessInputWithEntities(test::CreateTransactionEntityRange, 1);
	}

	TEST(TEST_CLASS, CanProcessInputWithSingleBlock) {
		AssertCanProcessInputWithEntities(test::CreateBlockEntityRange, 1);
	}

	TEST(TEST_CLASS, CanProcessInputWithMultipleTransactions) {
		AssertCanProcessInputWithEntities(test::CreateTransactionEntityRange, 3);
	}

	TEST(TEST_CLASS, CanProcessInputWithMultipleBlocks) {
		AssertCanProcessInputWithEntities(test::CreateBlockEntityRange, 3);
	}

	TEST(TEST_CLASS, CanProcessMultipleInputs) {
		RunAuditConsumerTest([](const auto& consumer, const auto& auditDirectory) {
			// Arrange:
			auto range1 = test::CreateTransactionEntityRange(1);
			auto range2 = test::CreateBlockEntityRange(1);
			auto range3 = test::CreateTransactionEntityRange(1);
			auto range4 = test::CreateBlockEntityRange(1);
			auto keys = test::GenerateRandomDataVector<Key>(4);

			auto rangeCopy1 = decltype(range1)::CopyRange(range1);
			auto rangeCopy2 = decltype(range2)::CopyRange(range2);
			auto rangeCopy3 = decltype(range3)::CopyRange(range3);
			auto rangeCopy4 = decltype(range4)::CopyRange(range4);

			// Act:
			auto result1 = consumer(MakeConsumerInput(std::move(range1), keys[0], InputSource::Remote_Pull));
			auto result2 = consumer(MakeConsumerInput(std::move(range2), keys[1], InputSource::Remote_Push));
			auto result3 = consumer(MakeConsumerInput(std::move(range3), keys[2], InputSource::Local));
			auto result4 = consumer(MakeConsumerInput(std::move(range4), keys[3], InputSource::Remote_Pull));

			// Assert:
			test::AssertContinued(result1);
			test::AssertContinued(result2);
			test::AssertContinued(result3);
			test::AssertContinued(result4);

			auto auditDirectoryPath = std::filesystem::path(auditDirectory);
			AssertFileContents(auditDirectoryPath / "1", InputSource::Remote_Pull, keys[0], *rangeCopy1.cbegin());
			AssertFileContents(auditDirectoryPath / "2", InputSource::Remote_Push, keys[1], *rangeCopy2.cbegin());
			AssertFileContents(auditDirectoryPath / "3", InputSource::Local, keys[2], *rangeCopy3.cbegin());
			AssertFileContents(auditDirectoryPath / "4", InputSource::Remote_Pull, keys[3], *rangeCopy4.cbegin());
		});
	}
}}
