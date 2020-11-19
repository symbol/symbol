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

#include "catapult/deltaset/BaseSetCommitPolicy.h"
#include "tests/test/other/DeltaElementsTestUtils.h"
#include "tests/test/other/UpdateSetTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS BaseSetCommitPolicyTests

	namespace {
		using Types = test::DeltaElementsTestUtils::Types;

		struct MemoryStorageTraits {
		public:
			struct TestContext : public test::DeltaElementsTestUtils::Wrapper<Types::MemoryMapType> {
			public:
				Types::StorageMapType Set;

			public:
				TestContext() {
					// seed the set with a few elements
					AddElement(Set, "aaa", 1);
					AddElement(Set, "ccc", 3);
					AddElement(Set, "ddd", 2);
				}
			};

		public:
			using CommitPolicy = BaseSetCommitPolicy<Types::StorageTraits>;

			template<typename TMap>
			static void AddElement(TMap& map, const std::string& name, unsigned int value, size_t dummy = 0) {
				map.emplace(std::make_pair(name, value), test::MutableTestElement(name, value)).first->second.Dummy = dummy;
			}

			static bool Contains(const Types::StorageMapType& map, const std::string& name, unsigned int value, size_t dummy = 0) {
				auto iter = map.find(std::make_pair(name, value));
				return map.cend() != iter && dummy == iter->second.Dummy;
			}
		};
	}

	DEFINE_UPDATE_SET_TESTS(MemoryStorageTraits)
	DEFINE_MEMORY_ONLY_UPDATE_SET_TESTS(MemoryStorageTraits)
}}
