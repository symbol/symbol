#include "catapult/plugins/PluginManager.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	// region basic

	TEST(PluginManagerTests, CanCreateManager) {
		// Act:
		auto config = model::BlockChainConfiguration::Uninitialized();
		config.BlockPruneInterval = 15;
		PluginManager manager(config);

		// Assert: compare BlockPruneInterval as a sentinel value because the manager copies the config
		EXPECT_EQ(15u, manager.config().BlockPruneInterval);
	}

	// endregion

	// region tx plugins

	TEST(PluginManagerTests, CanRegisterCustomTransactions) {
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

	TEST(PluginManagerTests, CanRegisterCustomCaches) {
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

	TEST(PluginManagerTests, CanRegisterCustomDiagnosticHandlers) {
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

	TEST(PluginManagerTests, CanRegisterCustomDiagnosticCounters) {
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

	// region validators

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

			[[noreturn]]
			validators::ValidationResult validate(const TNotification&, TArgs&&...) const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
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

	TEST(PluginManagerTests, CanRegisterStatelessValidators) {
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

	TEST(PluginManagerTests, CanRegisterStatefulValidators) {
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

	TEST(PluginManagerTests, CanRegisterObservers_PermanentOnly) {
		// Arrange:
		RunObserverTest([](const auto& manager) {
			// Act:
			auto pObserver = manager.createPermanentObserver();

			// Assert:
			auto expectedNames = std::vector<std::string>{ "alpha", "beta", "gamma" };
			EXPECT_EQ(expectedNames, pObserver->names());
		});
	}

	TEST(PluginManagerTests, CanObservers_All) {
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
}}
