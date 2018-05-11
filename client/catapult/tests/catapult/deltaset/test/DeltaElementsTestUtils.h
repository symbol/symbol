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

#pragma once
#include "BaseSetTestsInclude.h"
#include "catapult/deltaset/DeltaElements.h"

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
			/// Added elements.
			TSet Added;

			/// Removed elements.
			TSet Removed;

			/// Copied elements.
			TSet Copied;

		public:
			/// Returns a delta elements around the sub sets.
			auto deltas() {
				return deltaset::DeltaElements<TSet>(Added, Removed, Copied);
			}
		};
	};
}}
