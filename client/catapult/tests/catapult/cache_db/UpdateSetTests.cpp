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

#include "catapult/cache_db/UpdateSet.h"
#include "catapult/deltaset/BaseSetCommitPolicy.h"
#include "tests/catapult/cache_db/test/RdbTestUtils.h"
#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"
#include "tests/catapult/deltaset/test/DeltaElementsTestUtils.h"
#include "tests/catapult/deltaset/test/UpdateSetTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS UpdateSetTests

	namespace {
		struct TestValue {
		public:
			std::string KeyCopy;
			unsigned int Data;
			size_t Marker;
		};

		auto stoui(const std::string& str) {
			return static_cast<unsigned int>(std::stoul(str));
		}

		struct TestDescriptor {
		public:
			using KeyType = std::string;
			using ValueType = TestValue;
			using StorageType = std::pair<const KeyType, ValueType>;

			struct Serializer {
			public:
				static RawBuffer SerializeKey(const KeyType& key) {
					return { reinterpret_cast<const uint8_t*>(key.data()), key.size() };
				}

				static std::string SerializeValue(const StorageType& element) {
					std::ostringstream out;
					const auto& value = element.second;
					out << value.Data << "," << value.Marker << "," << value.KeyCopy;
					return out.str();
				}

				static ValueType DeserializeValue(const RawBuffer& buffer) {
					std::string input(reinterpret_cast<const char*>(buffer.pData), buffer.Size);
					auto comma1 = input.find(',');
					auto comma2 = input.find(',', comma1 + 1);
					auto intStr = input.substr(0, comma1);
					auto markerStr = input.substr(comma1 + 1, comma2 - comma1 - 1);
					auto keyCopy = input.substr(comma2 + 1);

					return { keyCopy, stoui(intStr), std::stoul(markerStr) };
				}
			};

			static const KeyType& GetKeyFromElement(const StorageType& element) {
				return element.first;
			}

			static const KeyType& GetKeyFromValue(const ValueType& value) {
				return value.KeyCopy;
			}
		};

		struct Types {
		private:
			using ElementType = TestDescriptor::ValueType;

			struct TestElementToKeyConverter {
				// ToKey is not needed, it's not used anywhere and clang would complain
			};

		public:
			using StorageMapType = RdbTypedColumnContainer<TestDescriptor>;
			using MemoryMapType = std::map<TestDescriptor::KeyType, TestDescriptor::ValueType>;

			using StorageTraits = deltaset::MapStorageTraits<StorageMapType, TestElementToKeyConverter, MemoryMapType>;
		};

		struct RdbStorageTraits {
		public:
			struct TestContext : public test::DeltaElementsTestUtils::Wrapper<Types::MemoryMapType> {
			private:
				test::RdbTestContext Context;

			public:
				Types::StorageMapType Set;

			public:
				TestContext()
						: Context({})
						, Set(Context.database(), 0) {
					// seed the set with a few elements
					AddElement(Set, "aaa", 1);
					AddElement(Set, "ccc", 3);
					AddElement(Set, "ddd", 2);
					Set.saveSize(3);
				}
			};

		private:
			static auto MakeElement(const std::string& name, unsigned int value, size_t marker) {
				return std::make_pair(name, TestValue{ name, value, marker });
			}

		public:
			using CommitPolicy = deltaset::BaseSetCommitPolicy<Types::StorageTraits>;

			template<typename TMap>
			static void AddElement(TMap& map, const std::string& name, unsigned int value, size_t dummy = 0) {
				map.insert(MakeElement(name, value, dummy));
			}

			static bool Contains(Types::StorageMapType& map, const std::string& name, unsigned int value, size_t dummy = 0) {
				auto iter = map.find(name);
				return map.cend() != iter && value == iter->second.Data && dummy == iter->second.Marker;
			}
		};
	}

	DEFINE_UPDATE_SET_TESTS(RdbStorageTraits)
}}
