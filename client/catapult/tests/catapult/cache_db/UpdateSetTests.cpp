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

#include "catapult/cache_db/UpdateSet.h"
#include "catapult/deltaset/BaseSetCommitPolicy.h"
#include "catapult/deltaset/PruningBoundary.h"
#include "tests/catapult/cache_db/test/BasicMapDescriptor.h"
#include "tests/catapult/cache_db/test/RdbTestUtils.h"
#include "tests/catapult/cache_db/test/StringKey.h"
#include "tests/test/other/DeltaElementsTestUtils.h"
#include "tests/test/other/UpdateSetTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS UpdateSetTests

	// region update set tests

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

		struct TestDescriptor : public test::BasicMapDescriptor<test::StringKey, TestValue> {
		public:
			struct Serializer {
			public:
				static std::string SerializeValue(const ValueType& value) {
					std::ostringstream out;
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
						: Context(RocksDatabaseSettings(
								test::TempDirectoryGuard::DefaultName(),
								{ "default" },
								FilterPruningMode::Disabled))
						, Set(Context.database(), 0) {
					// seed the set with a few elements
					AddElement(Set, "aaa", 1);
					AddElement(Set, "ccc", 3);
					AddElement(Set, "ddd", 2);
					Set.setSize(3);
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

	// endregion

	// region prune base set

	namespace {
		struct ColumnDescriptor {
		public:
			using KeyType = std::string;
			using ValueType = int;
			using StorageType = int;

			struct Serializer {
			public:
				static uint64_t KeyToBoundary(const KeyType& key) {
					return key.size();
				}
			};
		};

		enum class EntryPoint {
			Size,
			Prune,
			SetSize
		};

		struct MockDb {
		public:
			MockDb(size_t size, size_t numPruned)
					: Size(size)
					, NumPruned(numPruned)
			{}

		public:
			auto size() const {
				CallsOrder.push_back(EntryPoint::Size);
				return Size;
			}

			void setSize(size_t newSize) {
				CallsOrder.push_back(EntryPoint::SetSize);
				ParamSetSize = newSize;
			}

			auto prune(uint64_t pruningBoundary) {
				CallsOrder.push_back(EntryPoint::Prune);
				ParamPrune = pruningBoundary;
				return NumPruned;
			}

		public:
			const size_t Size;
			const size_t NumPruned;
			size_t ParamSetSize = 0;
			uint64_t ParamPrune = 0;
			mutable std::vector<EntryPoint> CallsOrder;
		};

		// pass-through into db
		struct MockContainer {
		public:
			MockContainer(MockDb& db, size_t) : m_db(db)
			{}

		public:
			auto size() const {
				return m_db.size();
			}

			void setSize(size_t newSize) {
				m_db.setSize(newSize);
			}

			size_t prune(uint64_t pruningBoundary) {
				return m_db.prune(pruningBoundary);
			}

		private:
			MockDb& m_db;
		};
	}

	TEST(TEST_CLASS, PruneBaseSetForwardsToStorage) {
		// Arrange:
		constexpr size_t Initial_Size = 543;
		constexpr size_t Num_Pruned = 123;
		MockDb db(Initial_Size, Num_Pruned);
		RdbTypedColumnContainer<ColumnDescriptor, MockContainer> container(db, 0);

		// Act:
		PruneBaseSet(container, deltaset::PruningBoundary<std::string>("hello world"));

		// Assert:
		// - order of calls
		// - mock container prune() was called with param returned from KeyToBoundary
		// - setSize was called with a proper difference
		EXPECT_EQ(std::vector<EntryPoint>({ EntryPoint::Size, EntryPoint::Prune, EntryPoint::SetSize }), db.CallsOrder);
		EXPECT_EQ(11u, db.ParamPrune);
		EXPECT_EQ(Initial_Size - Num_Pruned, db.ParamSetSize);
	}

	// endregion
}}
