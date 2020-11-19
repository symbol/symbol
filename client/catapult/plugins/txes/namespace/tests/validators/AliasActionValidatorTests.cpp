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

#include "src/validators/Validators.h"
#include "tests/test/plugins/DiscreteIntegerValidatorTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AliasActionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AliasAction,)

	namespace {
		struct AliasActionTraits {
			using EnumType = model::AliasAction;

			static constexpr auto Failure_Result = Failure_Namespace_Alias_Invalid_Action;
			static constexpr auto CreateValidator = CreateAliasActionValidator;

			static std::vector<uint8_t> ValidValues() {
				return { 0x00, 0x01 };
			}

			static std::vector<uint8_t> InvalidValues() {
				return { 0x02, 0xFF };
			}

			static auto CreateNotification(EnumType value) {
				return model::AliasLinkNotification(NamespaceId(), value);
			}
		};
	}

	DEFINE_DISCRETE_INTEGER_VALIDATOR_TESTS(TEST_CLASS, AliasActionTraits)
}}
