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

#include "src/config/SecretLockConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct SecretLockConfigurationTraits {
			using ConfigurationType = SecretLockConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "maxSecretLockDuration", "23'456d" },
							{ "minProofSize", "42" },
							{ "maxProofSize", "1234" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const SecretLockConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::BlockSpan(), config.MaxSecretLockDuration);
				EXPECT_EQ(0u, config.MinProofSize);
				EXPECT_EQ(0u, config.MaxProofSize);
			}

			static void AssertCustom(const SecretLockConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::BlockSpan::FromDays(23'456), config.MaxSecretLockDuration);
				EXPECT_EQ(42u, config.MinProofSize);
				EXPECT_EQ(1234u, config.MaxProofSize);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(SecretLockConfigurationTests, SecretLock)
}}
