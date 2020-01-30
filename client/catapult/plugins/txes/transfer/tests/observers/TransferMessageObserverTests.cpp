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

#include "src/observers/Observers.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS TransferMessageObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(TransferMessage, 0, Address(), config::CatapultDirectory(""))

	// region traits

	namespace {
		struct CommitTraits {
			static constexpr uint8_t Message_First_Byte = 0;
			static constexpr auto Notify_Mode = NotifyMode::Commit;
		};

		struct RollbackTraits {
			static constexpr uint8_t Message_First_Byte = 1;
			static constexpr auto Notify_Mode = NotifyMode::Rollback;
		};
	}

#define MESSAGE_OBSERVER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RollbackTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test context

	namespace {
		class TransferMessageObserverTestContext {
		public:
			explicit TransferMessageObserverTestContext(NotifyMode notifyMode) : m_notifyMode(notifyMode)
			{}

		public:
			size_t countFiles() {
				return test::CountFilesAndDirectories(m_tempDir.name());
			}

			uint64_t readIndexFile() const {
				return io::IndexFile((boost::filesystem::path(m_tempDir.name()) / "index.dat").generic_string()).get();
			}

			std::vector<uint8_t> readAll(const std::string& name) {
				io::RawFile dataFile((boost::filesystem::path(m_tempDir.name()) / name).generic_string(), io::OpenMode::Read_Only);
				std::vector<uint8_t> buffer(dataFile.size());
				dataFile.read(buffer);
				return buffer;
			}

		public:
			void observe(const Address& recipient, const model::TransferMessageNotification& notification) {
				auto pObserver = CreateTransferMessageObserver(0x1122334455667788, recipient, config::CatapultDirectory(m_tempDir.name()));

				test::ObserverTestContext context(m_notifyMode);
				test::ObserveNotification(*pObserver, notification, context);
			}

		private:
			NotifyMode m_notifyMode;
			test::TempDirectoryGuard m_tempDir;
		};
	}

	// endregion

	// region filtered - no files created

	namespace {
		template<typename TTraits>
		void AssertInvalidMarker(const std::vector<uint8_t>& message) {
			// Arrange:
			TransferMessageObserverTestContext context(TTraits::Notify_Mode);

			auto sender = test::GenerateRandomByteArray<Key>();
			auto recipient = test::GenerateRandomByteArray<Address>();
			auto unresolvedRecipient = test::UnresolveXor(recipient);
			auto messageSize = static_cast<uint16_t>(message.size());
			auto notification = model::TransferMessageNotification(sender, unresolvedRecipient, messageSize, message.data());

			// Act:
			context.observe(recipient, notification);

			// Assert:
			EXPECT_EQ(0u, context.countFiles());
		}
	}

	MESSAGE_OBSERVER_TRAITS_BASED_TEST(NoMessageIsWrittenWhenMessagePayloadIsLessThanMarkerSize) {
		AssertInvalidMarker<TTraits>({ 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22 });
	}

	MESSAGE_OBSERVER_TRAITS_BASED_TEST(NoMessageIsWrittenWhenMessagePayloadOnlyContainsMarker) {
		AssertInvalidMarker<TTraits>({ 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 });
	}

	MESSAGE_OBSERVER_TRAITS_BASED_TEST(NoMessageIsWrittenWhenMarkerDoesNotMatch) {
		AssertInvalidMarker<TTraits>({ 0x88, 0x77, 0x66, 0x5A, 0x44, 0x33, 0x22, 0x11, 0xAB, 0xCD, 0x82 });
	}

	MESSAGE_OBSERVER_TRAITS_BASED_TEST(NoMessageIsWrittenWhenRecipientDoesNotMatch) {
		// Arrange:
		TransferMessageObserverTestContext context(TTraits::Notify_Mode);

		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomByteArray<Address>();
		auto unresolvedRecipient = test::UnresolveXor(recipient);
		auto message = std::vector<uint8_t>{ 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xAB, 0xCD, 0x82 };
		auto messageSize = static_cast<uint16_t>(message.size());
		auto notification = model::TransferMessageNotification(sender, unresolvedRecipient, messageSize, message.data());

		// Act:
		context.observe(test::GenerateRandomByteArray<Address>(), notification);

		// Assert:
		EXPECT_EQ(0u, context.countFiles());
	}

	// endregion

	// region not filtered - files created

	MESSAGE_OBSERVER_TRAITS_BASED_TEST(MessageIsWrittenWhenMarkerAndRecipientBothMatch) {
		// Arrange:
		TransferMessageObserverTestContext context(TTraits::Notify_Mode);

		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomByteArray<Address>();
		auto unresolvedRecipient = test::UnresolveXor(recipient);
		auto message = std::vector<uint8_t>{ 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xAB, 0xCD, 0x82 };
		auto messageSize = static_cast<uint16_t>(message.size());
		auto notification = model::TransferMessageNotification(sender, unresolvedRecipient, messageSize, message.data());

		// Act:
		context.observe(recipient, notification);

		// Assert:
		EXPECT_EQ(2u, context.countFiles());
		EXPECT_EQ(1u, context.readIndexFile());

		auto expectedMessagePayloadSize = 3u;
		auto messageFileContents = context.readAll("0000000000000000.dat");
		ASSERT_EQ(1 + expectedMessagePayloadSize, messageFileContents.size());
		EXPECT_EQ(TTraits::Message_First_Byte, messageFileContents[0]);
		EXPECT_EQ_MEMORY(&message[sizeof(uint64_t)], &messageFileContents[1], expectedMessagePayloadSize);
	}

	// endregion
}}
