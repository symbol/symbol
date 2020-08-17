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

#include "catapult/subscribers/BrokerMessageReaders.h"
#include "catapult/io/PodIoUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS BrokerMessageReadersTests

	namespace {
		// region MockBufferSubscriber

		enum class Breadcrumb { Notify, Flush };

		class MockBufferSubscriberWithoutFlush {
		public:
			const auto& breadcrumbs() const {
				return m_breadcrumbs;
			}

			const auto& notifications() const {
				return m_notificationBuffers;
			}

		public:
			void notify(const std::vector<uint8_t>& buffer) {
				m_breadcrumbs.push_back(Breadcrumb::Notify);
				m_notificationBuffers.push_back(buffer);
			}

		protected:
			std::vector<Breadcrumb> m_breadcrumbs;

		private:
			std::vector<std::vector<uint8_t>> m_notificationBuffers;
		};

		class MockBufferSubscriber : public MockBufferSubscriberWithoutFlush {
		public:
			void flush() {
				m_breadcrumbs.push_back(Breadcrumb::Flush);
			}
		};

		void ReadNextBuffer(io::InputStream& inputStream, MockBufferSubscriberWithoutFlush& subscriber) {
			auto bufferSize = io::Read8(inputStream);
			std::vector<uint8_t> buffer;
			buffer.resize(bufferSize);
			inputStream.read(buffer);
			subscriber.notify(buffer);
		}

		void WriteNotificationBuffer(io::OutputStream& outputStream, const std::vector<uint8_t>& buffer) {
			io::Write8(outputStream, static_cast<uint8_t>(buffer.size()));
			outputStream.write(buffer);
		}

		// endregion
	}

	// region traits

	namespace {
		struct SubscriberWithFlushTraits {
			using SubscriberType = MockBufferSubscriber;

			static std::vector<Breadcrumb> Decorate(std::vector<Breadcrumb>&& breadcrumbs) {
				breadcrumbs.push_back(Breadcrumb::Flush);
				return std::move(breadcrumbs);
			}
		};

		struct SubscriberWithoutFlushTraits {
			using SubscriberType = MockBufferSubscriberWithoutFlush;

			static std::vector<Breadcrumb> Decorate(std::vector<Breadcrumb>&& breadcrumbs) {
				return std::move(breadcrumbs);
			}
		};
	}

#define FLUSH_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithFlush) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SubscriberWithFlushTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithoutFlush) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SubscriberWithoutFlushTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region ReadAll (Stream)

	FLUSH_TRAITS_BASED_TEST(ReadAllStream_CanReadZero) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		typename TTraits::SubscriberType subscriber;

		// Act:
		ReadAll(stream, subscriber, ReadNextBuffer);

		// Assert:
		EXPECT_EQ(TTraits::Decorate({}), subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		EXPECT_TRUE(notifications.empty());
	}

	FLUSH_TRAITS_BASED_TEST(ReadAllStream_CanReadSingle) {
		// Arrange:
		auto notificationBuffer = test::GenerateRandomVector(141);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		WriteNotificationBuffer(stream, notificationBuffer);
		stream.seek(0);

		typename TTraits::SubscriberType subscriber;

		// Act:
		ReadAll(stream, subscriber, ReadNextBuffer);

		// Assert:
		EXPECT_EQ(TTraits::Decorate({ Breadcrumb::Notify }), subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		ASSERT_EQ(1u, notifications.size());
		EXPECT_EQ(notificationBuffer, notifications[0]);
	}

	FLUSH_TRAITS_BASED_TEST(ReadAllStream_CanReadMultiple) {
		// Arrange:
		auto notificationBuffer1 = test::GenerateRandomVector(141);
		auto notificationBuffer2 = test::GenerateRandomVector(129);
		auto notificationBuffer3 = test::GenerateRandomVector(144);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		WriteNotificationBuffer(stream, notificationBuffer1);
		WriteNotificationBuffer(stream, notificationBuffer2);
		WriteNotificationBuffer(stream, notificationBuffer3);
		stream.seek(0);

		typename TTraits::SubscriberType subscriber;

		// Act:
		ReadAll(stream, subscriber, ReadNextBuffer);

		// Assert:
		EXPECT_EQ(TTraits::Decorate({ Breadcrumb::Notify, Breadcrumb::Notify, Breadcrumb::Notify }), subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		ASSERT_EQ(3u, notifications.size());
		EXPECT_EQ(notificationBuffer1, notifications[0]);
		EXPECT_EQ(notificationBuffer2, notifications[1]);
		EXPECT_EQ(notificationBuffer3, notifications[2]);
	}

	// endregion

	// region ReadAll (FileQueue / MessageQueueDescriptor)

	namespace {
		class QueueTestContext {
		public:
			QueueTestContext()
					: m_tempDataDir("q")
					, m_reader(m_tempDataDir.name()) {
				io::FileQueueWriter writer(m_tempDataDir.name()); // force creation of index writer file
			}

		public:
			std::string queuePath() {
				return m_tempDataDir.name();
			}

			io::FileQueueReader& reader() {
				return m_reader;
			}

		public:
			void write(const std::vector<uint8_t>& buffer) {
				write(std::vector<std::vector<uint8_t>>{ buffer });
			}

			void write(const std::vector<std::vector<uint8_t>>& buffers) {
				io::FileQueueWriter writer(m_tempDataDir.name());
				for (const auto& buffer : buffers)
					WriteNotificationBuffer(writer, buffer);

				writer.flush();
			}

		private:
			test::TempDirectoryGuard m_tempDataDir;
			io::FileQueueReader m_reader;
		};

		struct ReadAllFileQueueTraits {
			template<typename TSubscriber, typename TMessageReader>
			static void ReadAll(QueueTestContext& context, TSubscriber& subscriber, TMessageReader readNextMessage) {
				return subscribers::ReadAll(context.reader(), subscriber, readNextMessage);
			}
		};

		struct ReadAllMessageQueueDescriptorTraits {
			template<typename TSubscriber, typename TMessageReader>
			static void ReadAll(QueueTestContext& context, TSubscriber& subscriber, TMessageReader readNextMessage) {
				return subscribers::ReadAll({ context.queuePath(), "index_r.dat", "index.dat" }, subscriber, readNextMessage);
			}
		};
	}

#define READ_ALL_FILE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_FileQueue) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReadAllFileQueueTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MessageQueueDescriptor) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReadAllMessageQueueDescriptorTraits>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	READ_ALL_FILE_BASED_TEST(ReadAllFileQueue_CanReadZero) {
		// Arrange:
		QueueTestContext context;

		MockBufferSubscriber subscriber;

		// Act:
		TTraits::ReadAll(context, subscriber, ReadNextBuffer);

		// Assert:
		EXPECT_EQ(std::vector<Breadcrumb>(), subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		EXPECT_TRUE(notifications.empty());
	}

	READ_ALL_FILE_BASED_TEST(ReadAllFileQueue_CanReadSingle) {
		// Arrange:
		auto notificationBuffer = test::GenerateRandomVector(141);
		QueueTestContext context;
		context.write(notificationBuffer);

		MockBufferSubscriber subscriber;

		// Act:
		TTraits::ReadAll(context, subscriber, ReadNextBuffer);

		// Assert:
		EXPECT_EQ(std::vector<Breadcrumb>({ Breadcrumb::Notify, Breadcrumb::Flush }), subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		ASSERT_EQ(1u, notifications.size());
		EXPECT_EQ(notificationBuffer, notifications[0]);
	}

	READ_ALL_FILE_BASED_TEST(ReadAllFileQueue_CanReadMultiple) {
		// Arrange:
		auto notificationBuffer1 = test::GenerateRandomVector(141);
		auto notificationBuffer2 = test::GenerateRandomVector(129);
		auto notificationBuffer3 = test::GenerateRandomVector(144);

		QueueTestContext context;
		context.write(notificationBuffer1);
		context.write(notificationBuffer2);
		context.write(notificationBuffer3);

		MockBufferSubscriber subscriber;

		// Act:
		TTraits::ReadAll(context, subscriber, ReadNextBuffer);

		// Assert:
		std::vector<Breadcrumb> expectedBreadcrumbs{
			Breadcrumb::Notify, Breadcrumb::Flush,
			Breadcrumb::Notify, Breadcrumb::Flush,
			Breadcrumb::Notify, Breadcrumb::Flush
		};
		EXPECT_EQ(expectedBreadcrumbs, subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		ASSERT_EQ(3u, notifications.size());
		EXPECT_EQ(notificationBuffer1, notifications[0]);
		EXPECT_EQ(notificationBuffer2, notifications[1]);
		EXPECT_EQ(notificationBuffer3, notifications[2]);
	}

	READ_ALL_FILE_BASED_TEST(ReadAllFileQueue_CanReadMultipleWithMultipleNotificationsPerFile) {
		// Arrange:
		auto notificationBuffer1 = test::GenerateRandomVector(141);
		auto notificationBuffer2 = test::GenerateRandomVector(132);
		auto notificationBuffer3 = test::GenerateRandomVector(144);
		auto notificationBuffer4 = test::GenerateRandomVector(141);
		auto notificationBuffer5 = test::GenerateRandomVector(129);
		auto notificationBuffer6 = test::GenerateRandomVector(146);

		QueueTestContext context;
		context.write({ notificationBuffer1, notificationBuffer2 });
		context.write(notificationBuffer3);
		context.write({ notificationBuffer4, notificationBuffer5, notificationBuffer6 });

		MockBufferSubscriber subscriber;

		// Act:
		TTraits::ReadAll(context, subscriber, ReadNextBuffer);

		// Assert:
		std::vector<Breadcrumb> expectedBreadcrumbs{
			Breadcrumb::Notify, Breadcrumb::Notify, Breadcrumb::Flush,
			Breadcrumb::Notify, Breadcrumb::Flush,
			Breadcrumb::Notify, Breadcrumb::Notify, Breadcrumb::Notify, Breadcrumb::Flush
		};
		EXPECT_EQ(expectedBreadcrumbs, subscriber.breadcrumbs());

		const auto& notifications = subscriber.notifications();
		ASSERT_EQ(6u, notifications.size());
		EXPECT_EQ(notificationBuffer1, notifications[0]);
		EXPECT_EQ(notificationBuffer2, notifications[1]);
		EXPECT_EQ(notificationBuffer3, notifications[2]);
		EXPECT_EQ(notificationBuffer4, notifications[3]);
		EXPECT_EQ(notificationBuffer5, notifications[4]);
		EXPECT_EQ(notificationBuffer6, notifications[5]);
	}

	// endregion
}}
