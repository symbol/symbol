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
#include "catapult/deltaset/BaseSetDefaultTraits.h"
#include <string>

namespace catapult { namespace test {

	// region TestElement

	/// Test element used as the base of both immutable and mutable test elements.
	struct TestElement {
	public:
		TestElement() : TestElement("TestElement", 0)
		{}

		explicit TestElement(const std::string& name) : TestElement(name, 0)
		{}

		explicit TestElement(unsigned int value) : TestElement("TestElement", value)
		{}

		TestElement(const std::string& name, unsigned int value)
				: Name(name)
				, Value(value)
				, Hash(calculateHash())
				, Dummy(0)
				, HasherCallCount(0)
		{}

	public:
		bool operator<(const TestElement& rhs) const {
			return Name < rhs.Name || (Name == rhs.Name && Value < rhs.Value);
		}

		bool operator==(const TestElement& rhs) const {
			return Name == rhs.Name && Value == rhs.Value;
		}

	public:
		friend std::ostream& operator<<(std::ostream& out, const TestElement& element) {
			out << "TestElement(" << element.Name << ", " << element.Value << ")" << std::endl;
			return out;
		}

	public:
		size_t calculateHash() {
			auto raw = Name + std::to_string(Value);
			size_t checksum = 0;
			for (auto ch : raw)
				checksum += static_cast<size_t>(ch);

			return checksum;
		}

	public:
		std::string Name;
		unsigned int Value;
		size_t Hash;
		mutable size_t Dummy;
		mutable size_t HasherCallCount;
	};

	/// Immutable test element (deltas should not copy on write).
	struct ImmutableTestElement : public TestElement {
		using TestElement::TestElement;
	};

	/// Mutable test element (deltas should copy on write).
	struct MutableTestElement : public TestElement {
		using TestElement::TestElement;

		void mutate()
		{}
	};

	// endregion

	// region hashers and other helper operators

	/// Hashing function for TestElement.
	template<typename T>
	struct Hasher {
		size_t operator()(const T& element) const {
			++element.HasherCallCount;
			return element.Hash;
		}
	};

	template<typename T>
	struct Hasher<std::shared_ptr<T>> {
		size_t operator()(const std::shared_ptr<T>& pElement) const {
			Hasher<T> hasher;
			return hasher(*pElement);
		}
	};

	/// Checks two objects of the same type for equality.
	template<typename T>
	struct EqualityChecker {
		bool operator()(const T& lhs, const T& rhs) const {
			return lhs == rhs;
		}
	};

	template<typename T>
	struct EqualityChecker<std::shared_ptr<T>> {
		bool operator()(const std::shared_ptr<T>& pLhs, const std::shared_ptr<T>& pRhs) const {
			EqualityChecker<T> equalityChecker;
			return equalityChecker(*pLhs, *pRhs);
		}
	};

	/// Reverse comparator that reverses the natural sorting order of elements.
	template<typename T>
	struct ReverseComparator {
		bool operator()(const T& lhs, const T& rhs) const {
			return rhs < lhs;
		}
	};

	template<typename T>
	struct ReverseComparator<std::shared_ptr<T>> {
		bool operator()(const std::shared_ptr<T>& pLhs, const std::shared_ptr<T>& pRhs) const {
			ReverseComparator<T> comparator;
			return comparator(*pLhs, *pRhs);
		}
	};

	struct MapKeyHasher {
		size_t operator()(const std::pair<std::string, unsigned int>& pair) const {
			std::hash<std::string> stringHasher;
			std::hash<unsigned int> intHasher;
			return stringHasher(pair.first) ^ intHasher(pair.second);
		}
	};

	// endregion

	// region element to key converter

	template<typename T>
	struct TestElementToKeyConverter {
		static auto ToKey(const T& element) {
			return std::make_pair(element.Name, element.Value);
		}
	};

	template<typename T>
	struct TestElementToKeyConverter<std::shared_ptr<T>> {
		static auto ToKey(const std::shared_ptr<T>& pElement) {
			return std::make_pair(pElement->Name, pElement->Value);
		}
	};

	// endregion

	// region type declarations for set element type

	template<typename TMutabilityTraits>
	using SetElementType = std::remove_const_t<typename TMutabilityTraits::ElementType>;

	// endregion

	// region mutability traits

	using MutableElementValueTraits = deltaset::MutableTypeTraits<MutableTestElement>;
	using MutableElementPointerTraits = deltaset::MutableTypeTraits<std::shared_ptr<MutableTestElement>>;
	using ImmutableElementValueTraits = deltaset::ImmutableTypeTraits<const ImmutableTestElement>;
	using ImmutablePointerValueTraits = deltaset::ImmutableTypeTraits<std::shared_ptr<const ImmutableTestElement>>;

	// endregion
}}
