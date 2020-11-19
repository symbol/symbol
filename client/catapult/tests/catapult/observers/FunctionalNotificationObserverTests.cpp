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

#include "catapult/observers/FunctionalNotificationObserver.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS FunctionalNotificationObserverTests

	using NotificationType = model::AccountPublicKeyNotification;

	TEST(TEST_CLASS, HasCorrectName) {
		// Act:
		FunctionalNotificationObserverT<NotificationType> observer("Foo", [](const auto&, const auto&) {});

		// Assert:
		EXPECT_EQ("Foo", observer.name());
	}

	namespace {
		struct NotifyParams {
		public:
			NotifyParams(const NotificationType& notification, const ObserverContext& context)
					: pNotification(&notification)
					, pContext(&context)
			{}

		public:
			const NotificationType* pNotification;
			const ObserverContext* pContext;
		};
	}

	TEST(TEST_CLASS, NotifyDelegatesToFunction) {
		// Arrange:
		test::ParamsCapture<NotifyParams> capture;
		FunctionalNotificationObserverT<NotificationType> observer("Foo", [&](const auto& notification, const auto& context) {
			capture.push(notification, context);
		});

		// Act:
		cache::CatapultCache cache({});
		auto cacheDelta = cache.createDelta();
		auto context = test::CreateObserverContext(cacheDelta, Height(123), NotifyMode::Commit);

		auto publicKey = test::GenerateRandomByteArray<Key>();
		model::AccountPublicKeyNotification notification(publicKey);
		observer.notify(notification, context);

		// Assert:
		ASSERT_EQ(1u, capture.params().size());
		EXPECT_EQ(&notification, capture.params()[0].pNotification);
		EXPECT_EQ(&context, capture.params()[0].pContext);
	}
}}
