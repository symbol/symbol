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

#include "catapult/observers/DemuxObserverBuilder.h"
#include "tests/catapult/observers/test/AggregateObserverTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS DemuxObserverBuilderTests

	namespace {
		struct TestContext {
		public:
			std::vector<uint16_t> Breadcrumbs;
			AggregateNotificationObserverPointerT<model::Notification> pDemuxObserver;

		public:
			void notify(uint8_t notificationId, NotifyMode mode) {
				cache::CatapultCache cache({});
				auto cacheDelta = cache.createDelta();
				auto context = test::CreateObserverContext(cacheDelta, Height(123), mode);
				test::ObserveNotification<model::Notification>(*pDemuxObserver, test::TaggedNotification(notificationId), context);
			}
		};

		std::unique_ptr<TestContext> CreateTestContext(size_t numObservers, bool varyObservers = false) {
			auto pContext = std::make_unique<TestContext>();
			DemuxObserverBuilder builder;
			for (auto i = 0u; i < numObservers; ++i) {
				auto id = static_cast<uint8_t>(i + 1);
				if (!varyObservers || 1 == id % 2)
					builder.add(mocks::CreateTaggedBreadcrumbObserver(id, pContext->Breadcrumbs));
				else
					builder.add(mocks::CreateTaggedBreadcrumbObserver2(id, pContext->Breadcrumbs));
			}

			auto pDemuxObserver = builder.build();
			pContext->pDemuxObserver = std::move(pDemuxObserver);
			return pContext;
		}
	}

	// region basic delegation

	TEST(TEST_CLASS, CanCreateEmptyDemuxObserver) {
		// Act:
		auto pContext = CreateTestContext(0);
		pContext->notify(12, NotifyMode::Commit);

		// Assert:
		std::vector<std::string> expectedNames;
		EXPECT_EQ(0u, pContext->Breadcrumbs.size());
		EXPECT_EQ("{}", pContext->pDemuxObserver->name());
		EXPECT_EQ(expectedNames, pContext->pDemuxObserver->names());
	}

	TEST(TEST_CLASS, CanCreateDemuxObserverWithMultipleObservers) {
		// Act:
		auto pContext = CreateTestContext(10);
		pContext->notify(12, NotifyMode::Commit);

		// Assert:
		std::vector<std::string> expectedNames{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
		EXPECT_EQ(10u, pContext->Breadcrumbs.size());
		EXPECT_EQ("{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }", pContext->pDemuxObserver->name());
		EXPECT_EQ(expectedNames, pContext->pDemuxObserver->names());
	}

	TEST(TEST_CLASS, AddAllowsChaining) {
		// Arrange:
		auto pContext = std::make_unique<TestContext>();
		DemuxObserverBuilder builder;

		// Act:
		builder
			.add(mocks::CreateTaggedBreadcrumbObserver(2, pContext->Breadcrumbs))
			.add(mocks::CreateTaggedBreadcrumbObserver2(3, pContext->Breadcrumbs))
			.add(mocks::CreateTaggedBreadcrumbObserver(4, pContext->Breadcrumbs));
		pContext->pDemuxObserver = builder.build();

		// Act:
		pContext->notify(7, NotifyMode::Commit);

		// Assert:
		std::vector<uint16_t> expectedBreadcrumbs{ 0x0702, 0x0704 };
		EXPECT_EQ(2u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region commit + rollback

	TEST(TEST_CLASS, DemuxObserverNotifiesMatchingObserversOnCommit) {
		// Act:
		auto pContext = CreateTestContext(5, true);
		pContext->notify(4, NotifyMode::Commit);

		// Assert:
		std::vector<uint16_t> expectedBreadcrumbs{ 0x0401, 0x0403, 0x0405 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, DemuxObserverNotifiesMatchingObserversOnRollback) {
		// Act:
		auto pContext = CreateTestContext(5, true);
		pContext->notify(4, NotifyMode::Rollback);

		// Assert:
		std::vector<uint16_t> expectedBreadcrumbs{ 0x0405, 0x0403, 0x0401 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, DemuxObserverCanSendMultipleNotifications) {
		// Act:
		auto pContext = CreateTestContext(3, true);
		pContext->notify(4, NotifyMode::Commit);
		pContext->notify(2, NotifyMode::Rollback);
		pContext->notify(5, NotifyMode::Rollback);
		pContext->notify(3, NotifyMode::Commit);

		// Assert:
		std::vector<uint16_t> expectedBreadcrumbs{
			0x0401, 0x0403,
			0x0203, 0x0201,
			0x0503, 0x0501,
			0x0301, 0x0303
		};
		EXPECT_EQ(8u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region forwarding

	TEST(TEST_CLASS, NotificationsAreForwardedToChildObservers) {
		test::AssertNotificationsAreForwardedToChildObservers(DemuxObserverBuilder(), [](auto& builder, auto&& pObserver) {
			builder.template add<model::Notification>(std::move(pObserver));
		});
	}

	TEST(TEST_CLASS, ContextsAreForwardedToChildObservers) {
		test::AssertContextsAreForwardedToChildObservers(DemuxObserverBuilder(), [](auto& builder, auto&& pObserver) {
			builder.template add<model::Notification>(std::move(pObserver));
		});
	}

	// endregion

	// region filtering

	namespace {
		using Breadcrumbs = std::vector<std::string>;

		template<typename TNotification>
		class MockBreadcrumbObserver : public NotificationObserverT<TNotification> {
		public:
			MockBreadcrumbObserver(const std::string& name, Breadcrumbs& breadcrumbs)
					: m_name(name)
					, m_breadcrumbs(breadcrumbs)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			void notify(const TNotification&, ObserverContext&) const override {
				m_breadcrumbs.push_back(m_name);
			}

		private:
			std::string m_name;
			Breadcrumbs& m_breadcrumbs;
		};

		template<typename TNotification = model::Notification>
		NotificationObserverPointerT<TNotification> CreateBreadcrumbObserver(Breadcrumbs& breadcrumbs, const std::string& name) {
			return std::make_unique<MockBreadcrumbObserver<TNotification>>(name, breadcrumbs);
		}

		void AssertCanFilterObserversBasedOnNotificationType(const consumer<model::Notification&>& prepareNotification) {
			// Arrange:
			Breadcrumbs breadcrumbs;
			DemuxObserverBuilder builder;

			cache::CatapultCache cache({});
			auto cacheDelta = cache.createDelta();
			auto context = test::CreateObserverContext(cacheDelta, Height(123), NotifyMode::Commit);

			builder
				.add(CreateBreadcrumbObserver<model::AccountPublicKeyNotification>(breadcrumbs, "alpha"))
				.add(CreateBreadcrumbObserver<model::AccountAddressNotification>(breadcrumbs, "OMEGA"))
				.add(CreateBreadcrumbObserver(breadcrumbs, "zEtA"));
			auto pObserver = builder.build();

			// Act:
			auto notification = model::AccountPublicKeyNotification(Key());
			prepareNotification(notification);
			test::ObserveNotification<model::Notification>(*pObserver, notification, context);

			// Assert:
			Breadcrumbs expectedNames{ "alpha", "OMEGA", "zEtA" };
			EXPECT_EQ(expectedNames, pObserver->names());

			// - alpha matches notification type
			// - OMEGA does not match notification type
			// - zEtA matches all types
			Breadcrumbs expectedSelectedNames{ "alpha", "zEtA" };
			EXPECT_EQ(expectedSelectedNames, breadcrumbs);
		}
	}

	TEST(TEST_CLASS, CanFilterObserversBasedOnNotificationType) {
		AssertCanFilterObserversBasedOnNotificationType([](const auto&) {});
	}

	TEST(TEST_CLASS, CanFilterObserversBasedOnNotificationTypeIgnoringChannel) {
		AssertCanFilterObserversBasedOnNotificationType([](auto& notification) {
			// Arrange: change notification by changing channel
			model::SetNotificationChannel(notification.Type, model::NotificationChannel::None);
		});
	}

	// endregion
}}
