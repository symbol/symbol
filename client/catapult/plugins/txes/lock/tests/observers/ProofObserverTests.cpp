#include "src/observers/Observers.h"
#include "tests/test/LockStatusAndBalanceObserverTests.h"
#include "tests/test/plugins/ObserverTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS ProofObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::SecretLockInfoCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(Proof,)

	namespace {
		struct SecretTraits {
		public:
			using BasicTraits = test::BasicSecretLockInfoTestTraits;
			using NotificationBuilder = test::ProofNotificationBuilder;
			using ObserverTestContext = observers::ObserverTestContext;

			static auto CreateObserver() {
				return CreateProofObserver();
			}

			static auto DestinationAccount(const BasicTraits::ValueType& lockInfo) {
				return lockInfo.Recipient;
			}
		};
	}

	DEFINE_LOCK_STATUS_OBSERVER_TESTS(SecretTraits)
}}
