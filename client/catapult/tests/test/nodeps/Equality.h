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
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace test {

	/// Asserts that operator== returns the correct results when comparing values in \a descToValueMap against
	/// the default value (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// values equal to the value associated with \a defaultKey.
	/// \a equal is the comparison function used.
	template<typename TValue>
	void AssertEqualReturnsTrueForEqualObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TValue>& descToValueMap,
			const std::unordered_set<std::string>& equalityTags,
			const predicate<const TValue&, const TValue&>& equal) {
		// Arrange:
		const auto& defaultValue = descToValueMap.find(defaultKey)->second;
		for (const auto& value : descToValueMap) {
			auto isEqualityExpected = equalityTags.cend() != equalityTags.find(value.first);

			// Act:
			auto compareResult = equal(defaultValue, value.second);

			// Assert:
			EXPECT_EQ(isEqualityExpected, compareResult)
					<< "expected " << isEqualityExpected << " for '" << value.first << "'";
			CATAPULT_LOG(debug) << defaultKey << "' == '" << value.first << "' ? expected " << isEqualityExpected;
		}
	}

	/// Asserts that operator== returns the correct results when comparing values in \a descToValueMap against
	/// the default value (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// values equal to the value associated with \a defaultKey.
	template<typename TValue>
	void AssertOperatorEqualReturnsTrueForEqualObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TValue>& descToValueMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertEqualReturnsTrueForEqualObjects<TValue>(defaultKey, descToValueMap, equalityTags, [](const auto& lhs, const auto& rhs) {
			return lhs == rhs;
		});
	}

	/// Asserts that operator== returns the correct results when comparing values in \a descToValueMap against
	/// the default value (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// values equal to the value associated with \a defaultKey.
	template<typename TValue>
	void AssertOperatorEqualReturnsTrueForEqualObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, std::unique_ptr<TValue>>& descToValueMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertEqualReturnsTrueForEqualObjects<std::unique_ptr<TValue>>(defaultKey, descToValueMap, equalityTags, [](
				const auto& pLhs,
				const auto& pRhs) {
			return *pLhs == *pRhs;
		});
	}

	/// Asserts that operator!= returns the correct results when comparing values in \a descToValueMap against
	/// the default value (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// values equal to the value associated with \a defaultKey.
	/// \a notEqual is the comparison function used.
	template<typename TValue>
	void AssertNotEqualReturnsTrueForUnequalObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TValue>& descToValueMap,
			const std::unordered_set<std::string>& equalityTags,
			const predicate<const TValue&, const TValue&>& notEqual) {
		// Arrange:
		const auto& defaultValue = descToValueMap.find(defaultKey)->second;
		for (const auto& value : descToValueMap) {
			auto isEqualityExpected = equalityTags.cend() != equalityTags.find(value.first);

			// Act:
			auto compareResult = notEqual(defaultValue, value.second);

			// Assert:
			EXPECT_EQ(!isEqualityExpected, compareResult)
					<< "expected " << !isEqualityExpected << " for '" << value.first << "'";
			CATAPULT_LOG(debug) << defaultKey << "' != '" << value.first << "' ? expected " << !isEqualityExpected;
		}
	}

	/// Asserts that operator!= returns the correct results when comparing values in \a descToValueMap against
	/// the default value (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// values equal to the value associated with \a defaultKey.
	template<typename TValue>
	void AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TValue>& descToValueMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertNotEqualReturnsTrueForUnequalObjects<TValue>(defaultKey, descToValueMap, equalityTags, [](const auto& lhs, const auto& rhs) {
			return lhs != rhs;
		});
	}

	/// Asserts that operator!= returns the correct results when comparing values in \a descToValueMap against
	/// the default value (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// values equal to the value associated with \a defaultKey.
	template<typename TValue>
	void AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, std::unique_ptr<TValue>>& descToValueMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertNotEqualReturnsTrueForUnequalObjects<std::unique_ptr<TValue>>(defaultKey, descToValueMap, equalityTags, [](
				const auto& pLhs,
				const auto& pRhs) {
			return *pLhs != *pRhs;
		});
	}

	namespace detail {
		template<typename TValue>
		struct EqualityTestInputs {
			std::unordered_map<std::string, TValue> DescToValueMap;
			std::unordered_set<std::string> EqualityTags;
		};

		template<typename TValue>
		EqualityTestInputs<TValue> CreateEqualityTestInputsFromInitializerLists(
				std::initializer_list<TValue> equalValues,
				std::initializer_list<TValue> unequalValues) {
			EqualityTestInputs<TValue> inputs;

			size_t i = 0;
			for (const auto& value : equalValues) {
				++i;
				auto tag = std::to_string(i);
				inputs.DescToValueMap.emplace(tag, value);
				inputs.EqualityTags.emplace(tag);
			}

			for (const auto& value : unequalValues) {
				++i;
				auto tag = std::to_string(i);
				inputs.DescToValueMap.emplace(tag, value);
			}

			return inputs;
		}
	}

	/// Asserts that operator== returns \c true when comparing values in \a equalValues and \c false when comparing
	/// an value in \a equalValues with an value in \a unequalValues.
	template<typename TValue>
	void AssertOperatorEqualReturnsTrueForEqualObjects(
			std::initializer_list<TValue> equalValues,
			std::initializer_list<TValue> unequalValues) {
		auto inputs = detail::CreateEqualityTestInputsFromInitializerLists(equalValues, unequalValues);
		AssertOperatorEqualReturnsTrueForEqualObjects("1", inputs.DescToValueMap, inputs.EqualityTags);
	}

	/// Asserts that operator!= returns \c false when comparing values in \a equalValues and \c true when comparing
	/// an value in \a equalValues with an value in \a unequalValues.
	template<typename TValue>
	void AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			std::initializer_list<TValue> equalValues,
			std::initializer_list<TValue> unequalValues) {
		auto inputs = detail::CreateEqualityTestInputsFromInitializerLists(equalValues, unequalValues);
		AssertOperatorNotEqualReturnsTrueForUnequalObjects("1", inputs.DescToValueMap, inputs.EqualityTags);
	}
}}
