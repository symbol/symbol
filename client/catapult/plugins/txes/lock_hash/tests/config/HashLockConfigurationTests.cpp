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

#include "src/config/HashLockConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct HashLockConfigurationTraits {
			using ConfigurationType = HashLockConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "lockedFundsPerAggregate", "123'000'000" },
							{ "maxHashLockDuration", "12'345d" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const HashLockConfiguration& config) {
				// Assert:
				EXPECT_EQ(Amount(), config.LockedFundsPerAggregate);
				EXPECT_EQ(utils::BlockSpan(), config.MaxHashLockDuration);
			}

			static void AssertCustom(const HashLockConfiguration& config) {
				// Assert:
				EXPECT_EQ(Amount(123'000'000), config.LockedFundsPerAggregate);
				EXPECT_EQ(utils::BlockSpan::FromDays(12'345), config.MaxHashLockDuration);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(HashLockConfigurationTests, HashLock)
}}
