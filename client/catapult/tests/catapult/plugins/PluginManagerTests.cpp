#include "catapult/plugins/PluginManager.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

#define TEST_CLASS PluginManagerTests

	// region basic

	TEST(TEST_CLASS, CanCreateManager) {
		// Act:
		auto config = model::BlockChainConfiguration::Uninitialized();
		config.BlockPruneInterval = 15;
		PluginManager manager(config);

		// Assert: compare BlockPruneInterval as a sentinel value because the manager copies the config
		EXPECT_EQ(15u, manager.config().BlockPruneInterval);
	}

	// endregion

	// region tx plugins

	TEST(TEST_CLASS, CanRegisterCustomTransactions) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());

		// Act:
		for (auto i : { 7, 9, 4 })
			manager.addTransactionSupport(mocks::CreateMockTransactionPlugin(i));

		// Assert:
		EXPECT_EQ(3u, manager.transactionRegistry().size());

		for (auto i : { 7, 9, 4 }) {
			auto entityType = static_cast<model::EntityType>(i);
			EXPECT_TRUE(!!manager.transactionRegistry().findPlugin(entityType)) << "type " << i;
		}
	}

	// endregion

	// region cache plugins

	namespace {
		template<size_t CacheId>
		void AddSubCacheWithId(PluginManager& manager) {
			manager.addCacheSupport<test::SimpleCacheStorageTraits>(std::make_unique<test::SimpleCacheT<CacheId>>());
		}
	}

	TEST(TEST_CLASS, CanRegisterCustomCaches) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());

		// Act:
		AddSubCacheWithId<7>(manager);
		AddSubCacheWithId<9>(manager);
		AddSubCacheWithId<4>(manager);
		auto cache = manager.createCache();

		// Assert:
		EXPECT_EQ(3u, cache.storages().size());
		EXPECT_EQ(0u, cache.sub<test::SimpleCacheT<7>>().createView()->size());
		EXPECT_EQ(0u, cache.sub<test::SimpleCacheT<9>>().createView()->size());
		EXPECT_EQ(0u, cache.sub<test::SimpleCacheT<4>>().createView()->size());
	}

	// endregion

	// region diagnostic handler plugins

	TEST(TEST_CLASS, CanRegisterCustomDiagnosticHandlers) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());

		// Act:
		manager.addDiagnosticHandlerHook([](auto& handlers, const auto&) {
			handlers.registerHandler(static_cast<ionet::PacketType>(7), [](const auto&, const auto&) {});
			handlers.registerHandler(static_cast<ionet::PacketType>(9), [](const auto&, const auto&) {});
		});
		manager.addDiagnosticHandlerHook([](auto& handlers, const auto&) {
			handlers.registerHandler(static_cast<ionet::PacketType>(4), [](const auto&, const auto&) {});
		});

		ionet::ServerPacketHandlers handlers;
		manager.addDiagnosticHandlers(handlers, manager.createCache());

		// Assert:
		EXPECT_EQ(3u, handlers.size());
		for (auto type : { 7u, 9u, 4u }) {
			ionet::Packet packet;
			packet.Type = static_cast<ionet::PacketType>(type);
			EXPECT_TRUE(handlers.canProcess(packet)) << "packet type" << type;
		}
	}

	// endregion

	// region diagnostic counter handlers

	namespace {
		utils::DiagnosticCounter MakeDiagnosticCounter(uint64_t id) {
			return utils::DiagnosticCounter(utils::DiagnosticCounterId(id), []() { return 0; });
		}
	}

	TEST(TEST_CLASS, CanRegisterCustomDiagnosticCounters) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());

		// Act:
		manager.addDiagnosticCounterHook([](auto& counters, const auto&) {
			counters.push_back(MakeDiagnosticCounter(7));
			counters.push_back(MakeDiagnosticCounter(9));
		});
		manager.addDiagnosticCounterHook([](auto& counters, const auto&) {
			counters.push_back(MakeDiagnosticCounter(4));
		});

		std::vector<utils::DiagnosticCounter> counters;
		manager.addDiagnosticCounters(counters, manager.createCache());

		// Assert:
		ASSERT_EQ(3u, counters.size());
		EXPECT_EQ("G", counters[0].id().name());
		EXPECT_EQ("I", counters[1].id().name());
		EXPECT_EQ("D", counters[2].id().name());
	}

	// endregion

	// region validators - helpers

	namespace {
		template<typename TNotification, typename... TArgs>
		class NamedValidatorT : public validators::NotificationValidatorT<TNotification, TArgs...> {
		public:
			explicit NamedValidatorT(const std::string& name) : m_name(name)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			validators::ValidationResult validate(const TNotification&, TArgs&&...) const override {
				return validators::ValidationResult::Failure;
			}

		private:
			std::string m_name;
		};

		auto CreateNamedStatelessValidator(const std::string& name) {
			return std::make_unique<NamedValidatorT<model::Notification>>(name);
		}

		auto CreateNamedStatefulValidator(const std::string& name) {
			return std::make_unique<NamedValidatorT<model::Notification, const validators::ValidatorContext&>>(name);
		}
	}

	// endregion

	// region validators - stateless

	TEST(TEST_CLASS, CanRegisterStatelessValidators) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatelessValidator("alpha"));
			builder.add(CreateNamedStatelessValidator("beta"));
		});
		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatelessValidator("gamma"));
		});

		// Act:
		auto pValidator = manager.createStatelessValidator();

		// Assert:
		auto expectedNames = std::vector<std::string>{ "alpha", "beta", "gamma" };
		EXPECT_EQ(expectedNames, pValidator->names());
	}

	namespace {
		validators::ValidationResult ValidateStateless(const PluginManager& manager, bool suppress) {
			auto pValidator = suppress
					? manager.createStatelessValidator([](auto) { return true; })
					: manager.createStatelessValidator();
			auto notification = model::AccountPublicKeyNotification(test::GenerateRandomData<Key_Size>());
			return pValidator->validate(notification);
		}
	}

	TEST(TEST_CLASS, CanCreateStatelessValidatorWithNoSuppressedFailureFiltering) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatelessValidator("alpha"));
		});

		// Act: no suppression
		auto result = ValidateStateless(manager, false);

		// Assert:
		EXPECT_EQ(validators::ValidationResult::Failure, result);
	}

	TEST(TEST_CLASS, CanCreateStatelessValidatorWithCustomSuppressedFailureFiltering) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatelessValidator("alpha"));
		});

		// Act: suppress everything
		auto result = ValidateStateless(manager, true);

		// Assert:
		EXPECT_EQ(validators::ValidationResult::Success, result);
	}

	// endregion

	// region validators - stateful

	TEST(TEST_CLASS, CanRegisterStatefulValidators) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.addStatefulValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatefulValidator("alpha"));
			builder.add(CreateNamedStatefulValidator("beta"));
		});
		manager.addStatefulValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatefulValidator("gamma"));
		});

		// Act:
		auto pValidator = manager.createStatefulValidator();

		// Assert:
		auto expectedNames = std::vector<std::string>{ "alpha", "beta", "gamma" };
		EXPECT_EQ(expectedNames, pValidator->names());
	}

	namespace {
		validators::ValidationResult ValidateStateful(const PluginManager& manager, bool suppress) {
			auto pValidator = suppress
					? manager.createStatefulValidator([](auto) { return true; })
					: manager.createStatefulValidator();
			auto notification = model::AccountPublicKeyNotification(test::GenerateRandomData<Key_Size>());
			auto cache = cache::CatapultCache({});
			auto context = test::CreateValidatorContext(Height(123), cache.createView().toReadOnly());
			return pValidator->validate(notification, context);
		}
	}

	TEST(TEST_CLASS, CanCreateStatefulValidatorWithNoSuppressedFailureFiltering) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.addStatefulValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatefulValidator("alpha"));
		});

		// Act: no suppression
		auto result = ValidateStateful(manager, false);

		// Assert:
		EXPECT_EQ(validators::ValidationResult::Failure, result);
	}

	TEST(TEST_CLASS, CanCreateStatefulValidatorWithCustomSuppressedFailureFiltering) {
		// Arrange:
		PluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.addStatefulValidatorHook([](auto& builder) {
			builder.add(CreateNamedStatefulValidator("alpha"));
		});

		// Act: suppress everything
		auto result = ValidateStateful(manager, true);

		// Assert:
		EXPECT_EQ(validators::ValidationResult::Success, result);
	}

	// endregion

	// region observers

	namespace {
		class NamedObserver : public observers::NotificationObserverT<model::Notification> {
		public:
			explicit NamedObserver(const std::string& name) : m_name(name)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			void notify(const model::Notification&, const observers::ObserverContext&) const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

		private:
			std::string m_name;
		};

		observers::NotificationObserverPointerT<model::Notification> CreateNamedObserver(const std::string& name) {
			return std::make_unique<NamedObserver>(name);
		}

		template<typename TAction>
		void RunObserverTest(TAction action) {
			// Arrange:
			PluginManager manager(model::BlockChainConfiguration::Uninitialized());
			manager.addObserverHook([](auto& builder) {
				builder.add(CreateNamedObserver("alpha"));
				builder.add(CreateNamedObserver("beta"));
			});
			manager.addTransientObserverHook([](auto& builder) {
				builder.add(CreateNamedObserver("zeta"));
			});
			manager.addObserverHook([](auto& builder) {
				builder.add(CreateNamedObserver("gamma"));
			});
			manager.addTransientObserverHook([](auto& builder) {
				builder.add(CreateNamedObserver("omega"));
			});

			// Act:
			action(manager);
		}
	}

	TEST(TEST_CLASS, CanRegisterObservers_PermanentOnly) {
		// Arrange:
		RunObserverTest([](const auto& manager) {
			// Act:
			auto pObserver = manager.createPermanentObserver();

			// Assert:
			auto expectedNames = std::vector<std::string>{ "alpha", "beta", "gamma" };
			EXPECT_EQ(expectedNames, pObserver->names());
		});
	}

	TEST(TEST_CLASS, CanRegisterObservers_All) {
		// Arrange:
		RunObserverTest([](const auto& manager) {
			// Act:
			auto pObserver = manager.createObserver();

			// Assert: permanent observers run before transient observers
			auto expectedNames = std::vector<std::string>{ "alpha", "beta", "gamma", "zeta", "omega" };
			EXPECT_EQ(expectedNames, pObserver->names());
		});
	}

	// endregion

	// region notification publisher

	namespace {
		template<typename TPublisherFactory>
		void AssertCanCreateNotificationPublisher(size_t expectedNumNotifications, TPublisherFactory publisherFactory) {
			// Arrange:
			PluginManager manager(model::BlockChainConfiguration::Uninitialized());
			manager.addTransactionSupport(mocks::CreateMockTransactionPlugin());

			auto pTransaction = mocks::CreateMockTransaction(0);
			mocks::MockNotificationSubscriber subscriber;

			// Act: create a publisher and publish a transaction
			auto pPublisher = publisherFactory(manager);
			pPublisher->publish(*pTransaction, subscriber);

			// Assert: all expected notifications were raised
			EXPECT_EQ(expectedNumNotifications, subscriber.notificationTypes().size());
		}
	}

	TEST(TEST_CLASS, CanCreateDefaultNotificationPublisher) {
		// Assert: 4 basic and 1 custom notifications should be raised
		AssertCanCreateNotificationPublisher(5u, [](const auto& manager) {
			return manager.createNotificationPublisher();
		});
	}

	TEST(TEST_CLASS, CanCreateCustomNotificationPublisher) {
		// Assert: 4 basic notifications should be raised
		AssertCanCreateNotificationPublisher(4u, [](const auto& manager) {
			return manager.createNotificationPublisher(model::PublicationMode::Basic);
		});
	}

	// endregion
}}
