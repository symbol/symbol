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

#include "catapult/plugins/CacheHandlers.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

#define TEST_CLASS CacheHandlersTests

	namespace {
		struct CacheDescriptor {
			using CacheType = test::SimpleCacheT<2>;
			using CacheViewType = CacheType::CacheViewType;
			using KeyType = size_t;
			using ValueType = size_t;

			struct PatriciaTree {
				struct Serializer {
					static std::string SerializeValue(const ValueType&) {
						// stubbed out placeholder, which is adequate because only handler registration is tested below
						return std::string();
					}
				};
			};
		};

		template<typename TAction>
		void RunHandlerRegistrationTest(TAction action) {
			// Arrange:
			auto manager = test::CreatePluginManager();
			manager.addCacheSupport<test::SimpleCacheStorageTraits>(std::make_unique<CacheDescriptor::CacheType>());
			auto cache = manager.createCache();

			// Act:
			CacheHandlers<CacheDescriptor>::Register<static_cast<model::FacilityCode>(123)>(manager);

			// Assert:
			action(manager, cache);
		}
	}

	TEST(TEST_CLASS, CacheNonDiagnosticHandlersAreRegistered) {
		// Act:
		RunHandlerRegistrationTest([](auto& pluginManager, const auto& cache) {
			ionet::ServerPacketHandlers packetHandlers;
			pluginManager.addHandlers(packetHandlers, cache);

			// Assert:
			EXPECT_EQ(1u, packetHandlers.size());
			EXPECT_TRUE(packetHandlers.canProcess(static_cast<ionet::PacketType>(0x200 + 123)));
		});
	}

	TEST(TEST_CLASS, CacheDiagnosticHandlersAreRegistered) {
		// Act:
		RunHandlerRegistrationTest([](auto& pluginManager, const auto& cache) {
			ionet::ServerPacketHandlers packetHandlers;
			pluginManager.addDiagnosticHandlers(packetHandlers, cache);

			// Assert:
			EXPECT_EQ(1u, packetHandlers.size());
			EXPECT_TRUE(packetHandlers.canProcess(static_cast<ionet::PacketType>(0x400 + 123)));
		});
	}
}}
