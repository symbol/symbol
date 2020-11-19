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

#pragma once
#include "UnsupportedSerializer.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace test {

	/// Container of types used in cache tests.
	/// \note Using class to avoid polluting test namespace.
	class TestCacheTypes {
	public:
		// region TestHeightGroupedCacheDescriptor

		/// Identifier group that groups int values by height.
		class TestIdentifierGroup : public utils::IdentifierGroup<int, Height, std::hash<int>> {
		public:
#ifdef _MSC_VER
			TestIdentifierGroup() : TestIdentifierGroup(Height())
			{}
#endif

			using utils::IdentifierGroup<int, Height, std::hash<int>>::IdentifierGroup;
		};

		/// Cache descriptor for cache of values grouped by height.
		struct TestHeightGroupedCacheDescriptor {
			using KeyType = Height;
			using ValueType = TestIdentifierGroup;

			using Serializer = UnsupportedSerializer<KeyType, ValueType>;

			static KeyType GetKeyFromValue(const ValueType& value) {
				return value.key();
			}
		};

		// endregion

		// region Test(Activity)CacheDescriptor

		/// Pair composed of string and height that emulates values supporting activity.
		class StringHeightPair {
		public:
#ifdef _MSC_VER
			StringHeightPair() : StringHeightPair("", Height())
			{}
#endif

			StringHeightPair(const std::string& str, Height deactivateHeight)
					: m_str(str)
					, m_deactivateHeight(deactivateHeight)
			{}

		public:
			const std::string& str() const {
				return m_str;
			}

			bool isActive(Height height) const {
				return height < m_deactivateHeight;
			}

		private:
			std::string m_str;
			Height m_deactivateHeight;
		};

		/// Cache that indexes strings by size.
		struct TestCacheDescriptor {
			using KeyType = int;
			using ValueType = std::string;
			using Serializer = UnsupportedSerializer<KeyType, ValueType>;

			static constexpr auto Name = "TestCache";

			static KeyType GetKeyFromValue(const ValueType& value) {
				return static_cast<int>(value.size());
			}
		};

		/// Cache that indexes string height pairs by size.
		struct TestActivityCacheDescriptor {
			using KeyType = int;
			using ValueType = StringHeightPair;
			using Serializer = UnsupportedSerializer<KeyType, ValueType>;

			static constexpr auto Name = "TestActivityCache";

			static KeyType GetKeyFromValue(const ValueType& value) {
				return static_cast<int>(value.str().size());
			}
		};

		// endregion

		// region sets

	private:
		using HeightGroupedTypes = cache::MutableUnorderedMapAdapter<TestHeightGroupedCacheDescriptor, utils::BaseValueHasher<Height>>;
		using BasicTypes = cache::MutableUnorderedMapAdapter<TestCacheDescriptor>;
		using BasicActivityTypes = cache::MutableUnorderedMapAdapter<TestActivityCacheDescriptor>;

	public:
		/// Base set wrapper that holds a CacheDatabase.
		template<typename TBaseSet>
		class BaseSetTypeWrapper : public TBaseSet {
		public:
			BaseSetTypeWrapper() : TBaseSet(deltaset::ConditionalContainerMode::Memory, m_database, 0)
			{}

		private:
			cache::CacheDatabase m_database;
		};

		using HeightGroupedBaseSetType = BaseSetTypeWrapper<HeightGroupedTypes::BaseSetType>;
		using HeightGroupedBaseSetDeltaType = HeightGroupedTypes::BaseSetDeltaType;
		using BaseSetType = BaseSetTypeWrapper<BasicTypes::BaseSetType>;
		using BaseSetDeltaType = BasicTypes::BaseSetDeltaType;
		using BaseActivitySetType = BaseSetTypeWrapper<BasicActivityTypes::BaseSetType>;

		// endregion
	};
}}
