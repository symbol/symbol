#include "catapult/observers/FunctionalNotificationObserver.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {
	using NotificationType = model::AccountPublicKeyNotification;

	TEST(FunctionalNotificationObserverTests, HasCorrectName) {
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

	TEST(FunctionalNotificationObserverTests, NotifyDelegatesToFunction) {
		// Arrange:
		test::ParamsCapture<NotifyParams> capture;
		FunctionalNotificationObserverT<NotificationType> observer("Foo", [&](const auto& notification, const auto& context) {
			capture.push(notification, context);
		});

		// Act:
		state::CatapultState state;
		cache::CatapultCache cache({});
		auto cacheDelta = cache.createDelta();
		auto context = test::CreateObserverContext(cacheDelta, state, Height(123), NotifyMode::Commit);

		auto publicKey = test::GenerateRandomData<Key_Size>();
		model::AccountPublicKeyNotification notification(publicKey);
		observer.notify(notification, context);

		// Assert:
		ASSERT_EQ(1u, capture.params().size());
		EXPECT_EQ(&notification, capture.params()[0].pNotification);
		EXPECT_EQ(&context, capture.params()[0].pContext);
	}
}}
