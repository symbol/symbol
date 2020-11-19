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
#include "TestElement.h"
#include "catapult/deltaset/DeltaElements.h"
#include <map>
#include <unordered_map>

namespace catapult { namespace test {

	/// Test helpers for interacting with delta elements representing mutable element values with storage virtualization.
	class DeltaElementsTestUtils {
	public:
		/// Type definitions.
		struct Types {
		private:
			using ElementType = SetElementType<MutableElementValueTraits>;

		public:
			using StorageMapType = std::map<std::pair<std::string, unsigned int>, ElementType>;
			using MemoryMapType = std::unordered_map<std::pair<std::string, unsigned int>, ElementType, MapKeyHasher>;

			// to emulate storage virtualization, use two separate sets (ordered and unordered)
			using StorageTraits = deltaset::MapStorageTraits<StorageMapType, TestElementToKeyConverter<ElementType>, MemoryMapType>;
		};

	public:
		/// Storage for delta elements.
		template<typename TSet>
		struct Wrapper {
		public:
			using SetType = TSet;
			using MemorySetType = SetType;

		public:
			/// Added elements.
			SetType Added;

			/// Removed elements.
			SetType Removed;

			/// Copied elements.
			SetType Copied;

		public:
			/// Gets a delta elements around the sub sets.
			auto deltas() const {
				return deltaset::DeltaElements<SetType>(Added, Removed, Copied);
			}
		};

		/// Mixin that provides generational change emulation.
		template<typename TSet>
		class GenerationalChangeMixin {
		public:
			using KeyType = typename TSet::key_type;

		public:
			/// Creates mixin.
			GenerationalChangeMixin() : m_generationId(1)
			{}

		public:
			/// Gets the current generation id.
			uint32_t generationId() const {
				return m_generationId;
			}

			/// Gets the generation id associated with \a key.
			uint32_t generationId(const KeyType& key) const {
				// unlike BaseSetDelta, default generation is initial generation (1) instead of unset generation (0)
				auto iter = m_keyGenerationIdMap.find(key);
				return m_keyGenerationIdMap.cend() == iter ? 1 : iter->second;
			}

			/// Sets the generation id (\a generationId) for \a key.
			void setGenerationId(const KeyType& key, uint32_t generationId) {
				m_keyGenerationIdMap[key] = generationId;
			}

			/// Increments the generation id.
			void incrementGenerationId() {
				++m_generationId;
			}

		private:
			uint32_t m_generationId;
			std::unordered_map<KeyType, uint32_t> m_keyGenerationIdMap;
		};

		/// Storage for delta elements with generational support.
		template<typename TSet>
		struct WrapperWithGenerationalSupport
				: public Wrapper<TSet>
				, public GenerationalChangeMixin<TSet>
		{};
	};
}}
