#pragma once
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace test {

	/// Asserts that operator== returns the correct results when comparing entities in \a descToEntityMap against
	/// the default entity (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// entity values equal to the entity associated with \a defaultKey.
	/// \a equal is the comparison function used.
	template<typename TEntity>
	void AssertEqualReturnsTrueForEqualObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TEntity>& descToEntityMap,
			const std::unordered_set<std::string>& equalityTags,
			const std::function<bool (const TEntity&, const TEntity&)>& equal) {
		// Arrange:
		const auto& Default_Entity = descToEntityMap.find(defaultKey)->second;
		for (const auto& entity : descToEntityMap) {
			auto isEqualityExpected = equalityTags.cend() != equalityTags.find(entity.first);

			// Act:
			auto compareResult = equal(Default_Entity, entity.second);

			// Assert:
			EXPECT_EQ(isEqualityExpected, compareResult)
					<< "expected " << isEqualityExpected << " for '" << entity.first << "'";
			CATAPULT_LOG(debug) << defaultKey << "' == '" << entity.first << "' ? expected " << isEqualityExpected;
		}
	}

	/// Asserts that operator== returns the correct results when comparing entities in \a descToEntityMap against
	/// the default entity (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// entity values equal to the entity associated with \a defaultKey.
	template<typename TEntity>
	void AssertOperatorEqualReturnsTrueForEqualObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TEntity>& descToEntityMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertEqualReturnsTrueForEqualObjects<TEntity>(
				defaultKey,
				descToEntityMap,
				equalityTags,
				[](const auto& lhs, const auto& rhs) { return lhs == rhs; });
	}

	/// Asserts that operator== returns the correct results when comparing entities in \a descToEntityMap against
	/// the default entity (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// entity values equal to the entity associated with \a defaultKey.
	template<typename TEntity>
	void AssertOperatorEqualReturnsTrueForEqualObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, std::unique_ptr<TEntity>>& descToEntityMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertEqualReturnsTrueForEqualObjects<std::unique_ptr<TEntity>>(
				defaultKey,
				descToEntityMap,
				equalityTags,
				[](const auto& pLhs, const auto& pRhs) { return *pLhs == *pRhs; });
	}

	/// Asserts that operator!= returns the correct results when comparing entities in \a descToEntityMap against
	/// the default entity (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// entity values equal to the entity associated with \a defaultKey.
	/// \a notEqual is the comparison function used.
	template<typename TEntity>
	void AssertNotEqualReturnsTrueForUnequalObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TEntity>& descToEntityMap,
			const std::unordered_set<std::string>& equalityTags,
			const std::function<bool (const TEntity&, const TEntity&)>& notEqual) {
		// Arrange:
		const auto& Default_Entity = descToEntityMap.find(defaultKey)->second;
		for (const auto& entity : descToEntityMap) {
			auto isEqualityExpected = equalityTags.cend() != equalityTags.find(entity.first);

			// Act:
			auto compareResult = notEqual(Default_Entity, entity.second);

			// Assert:
			EXPECT_EQ(!isEqualityExpected, compareResult)
					<< "expected " << !isEqualityExpected << " for '" << entity.first << "'";
			CATAPULT_LOG(debug) << defaultKey << "' != '" << entity.first << "' ? expected " << !isEqualityExpected;
		}
	}

	/// Asserts that operator!= returns the correct results when comparing entities in \a descToEntityMap against
	/// the default entity (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// entity values equal to the entity associated with \a defaultKey.
	template<typename TEntity>
	void AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, TEntity>& descToEntityMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertNotEqualReturnsTrueForUnequalObjects<TEntity>(
				defaultKey,
				descToEntityMap,
				equalityTags,
				[](const auto& lhs, const auto& rhs) { return lhs != rhs; });
	}

	/// Asserts that operator!= returns the correct results when comparing entities in \a descToEntityMap against
	/// the default entity (having key \a defaultKey in the map). keys in \a equalityTags are expected to have
	/// entity values equal to the entity associated with \a defaultKey.
	template<typename TEntity>
	void AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			const std::string& defaultKey,
			const std::unordered_map<std::string, std::unique_ptr<TEntity>>& descToEntityMap,
			const std::unordered_set<std::string>& equalityTags) {
		AssertNotEqualReturnsTrueForUnequalObjects<std::unique_ptr<TEntity>>(
				defaultKey,
				descToEntityMap,
				equalityTags,
				[](const auto& pLhs, const auto& pRhs) { return *pLhs != *pRhs; });
	}

	namespace detail {
		template<typename TEntity>
		struct EqualityTestInputs {
			std::unordered_map<std::string, TEntity> DescToEntityMap;
			std::unordered_set<std::string> EqualityTags;
		};

		template<typename TEntity>
		EqualityTestInputs<TEntity> CreateEqualityTestInputsFromInitializerLists(
				std::initializer_list<TEntity> equalEntities,
				std::initializer_list<TEntity> unequalEntities) {
			EqualityTestInputs<TEntity> inputs;

			size_t i = 0;
			for (const auto& entity : equalEntities) {
				++i;
				auto tag = std::to_string(i);
				inputs.DescToEntityMap.emplace(tag, entity);
				inputs.EqualityTags.emplace(tag);
			}

			for (const auto& entity : unequalEntities) {
				++i;
				auto tag = std::to_string(i);
				inputs.DescToEntityMap.emplace(tag, entity);
			}

			return inputs;
		}
	}

	/// Asserts that operator== returns \c true when comparing entities in \a equalEntities and \c false when comparing
	/// an entity in \a equalEntities with an entity in \a unequalEntities.
	template<typename TEntity>
	void AssertOperatorEqualReturnsTrueForEqualObjects(
			std::initializer_list<TEntity> equalEntities,
			std::initializer_list<TEntity> unequalEntities) {
		auto inputs = detail::CreateEqualityTestInputsFromInitializerLists(equalEntities, unequalEntities);
		AssertOperatorEqualReturnsTrueForEqualObjects("1", inputs.DescToEntityMap, inputs.EqualityTags);
	}

	/// Asserts that operator!= returns \c false when comparing entities in \a equalEntities and \c true when comparing
	/// an entity in \a equalEntities with an entity in \a unequalEntities.
	template<typename TEntity>
	void AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			std::initializer_list<TEntity> equalEntities,
			std::initializer_list<TEntity> unequalEntities) {
		auto inputs = detail::CreateEqualityTestInputsFromInitializerLists(equalEntities, unequalEntities);
		AssertOperatorNotEqualReturnsTrueForUnequalObjects("1", inputs.DescToEntityMap, inputs.EqualityTags);
	}
}}
