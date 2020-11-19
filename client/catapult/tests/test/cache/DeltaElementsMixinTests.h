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
#include "tests/TestHarness.h"
#include <set>

namespace catapult { namespace test {

	/// Modification policy that calls insert.
	struct DeltaInsertModificationPolicy {
		static constexpr auto Is_Mutable = true;
		static constexpr auto Is_Strictly_Ordered = false;

		template<typename TDelta, typename TValue>
		static void Modify(TDelta& delta, const TValue& value) {
			delta.insert(value);
		}
	};

	/// Modification policy that calls remove and insert.
	struct DeltaRemoveInsertModificationPolicy {
		static constexpr auto Is_Mutable = true;
		static constexpr auto Is_Strictly_Ordered = false;

		template<typename TDelta, typename TValue>
		static void Modify(TDelta& delta, const TValue& value) {
			delta.remove(value);
			delta.insert(value);
		}
	};

	/// Delta elements mixin test suite.
	template<typename TTraits, typename TModificationPolicy>
	class DeltaElementsMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;
		using IdType = typename TTraits::IdType;
		using ValueType = typename TTraits::ValueType;

	public:
		static void AssertInitiallyNoElementsAreMarkedAsAddedOrModifiedOrRemoved() {
			// Arrange:
			CacheType cache;
			auto delta = cache.createDelta();

			// Assert:
			AssertMarkedElements(*delta, {}, {}, {});
		}

		static void AssertAddedElementsAreMarkedAsAdded() {
			// Arrange:
			CacheType cache;
			auto delta = cache.createDelta();

			// Act:
			delta->insert(TTraits::CreateWithId(123));

			// Assert:
			AssertMarkedElements(*delta, { TTraits::MakeId(123) }, {}, {});
		}

		static void AssertModifiedElementsAreMarkedAsModified() {
			// Arrange:
			CacheType cache;
			auto delta = cache.createDelta();
			delta->insert(TTraits::CreateWithId(123));
			cache.commit();

			// Act:
			TModificationPolicy::Modify(*delta, TTraits::CreateWithId(123));

			// Assert: if elements are immutable, no modifications are possible
			AssertMarkedElements(*delta, {}, SelectIfMutable({ TTraits::MakeId(123) }), {});
		}

		static void AssertRemovedElementsAreMarkedAsRemoved() {
			// Arrange:
			CacheType cache;
			auto delta = cache.createDelta();
			delta->insert(TTraits::CreateWithId(123));
			cache.commit();

			// Act:
			delta->remove(TTraits::MakeId(123));

			// Assert:
			AssertMarkedElements(*delta, {}, {}, { TTraits::MakeId(123) });
		}

		static void AssertMultipleMarkedElementsCanBeTracked() {
			// Arrange:
			CacheType cache;
			auto delta = cache.createDelta();
			for (uint8_t i = 100; i < 110; ++i)
				delta->insert(TTraits::CreateWithId(i));

			cache.commit();

			// Act + Assert:
			if (TModificationPolicy::Is_Strictly_Ordered)
				AssertMultipleMarkedElementsCanBeTrackedOrdered(*delta);
			else
				AssertMultipleMarkedElementsCanBeTrackedUnordered(*delta);
		}

	private:
		template<typename TDelta>
		static void AssertMultipleMarkedElementsCanBeTrackedOrdered(TDelta& delta) {
			// Act:
			// - add three
			delta.insert(TTraits::CreateWithId(110));
			delta.insert(TTraits::CreateWithId(111));
			delta.insert(TTraits::CreateWithId(112));

			// - modify one
			TModificationPolicy::Modify(delta, TTraits::CreateWithId(112));

			// - remove five
			delta.remove(TTraits::MakeId(112));
			delta.remove(TTraits::MakeId(111));
			delta.remove(TTraits::MakeId(110));
			delta.remove(TTraits::MakeId(109));
			delta.remove(TTraits::MakeId(108));

			// Assert:
			AssertMarkedElements(
					delta,
					{ TTraits::MakeId(110), TTraits::MakeId(111), TTraits::MakeId(112) },
					{},
					{ TTraits::MakeId(108), TTraits::MakeId(109), TTraits::MakeId(110), TTraits::MakeId(111), TTraits::MakeId(112) });
		}

		template<typename TDelta>
		static void AssertMultipleMarkedElementsCanBeTrackedUnordered(TDelta& delta) {
			// Act:
			// - add two
			delta.insert(TTraits::CreateWithId(123));
			delta.insert(TTraits::CreateWithId(128));

			// - modify three
			TModificationPolicy::Modify(delta, TTraits::CreateWithId(105));
			TModificationPolicy::Modify(delta, TTraits::CreateWithId(107));
			TModificationPolicy::Modify(delta, TTraits::CreateWithId(108));

			// - remove four
			delta.remove(TTraits::MakeId(100));
			delta.remove(TTraits::MakeId(101));
			delta.remove(TTraits::MakeId(104));
			delta.remove(TTraits::MakeId(106));

			// Assert: if elements are immutable, no modifications are possible
			AssertMarkedElements(
					delta,
					{ TTraits::MakeId(123), TTraits::MakeId(128) },
					SelectIfMutable({ TTraits::MakeId(105), TTraits::MakeId(107), TTraits::MakeId(108) }),
					{ TTraits::MakeId(100), TTraits::MakeId(101), TTraits::MakeId(104), TTraits::MakeId(106) });
		}

	private:
		// use set instead of unordered_set to simplify traits (all keys are required to support equality)

		static std::set<IdType> SelectIfMutable(const std::set<IdType>& ids) {
			return TModificationPolicy::Is_Mutable ? ids : std::set<IdType>();
		}

		template<typename TElementContainer>
		static std::set<IdType> CollectIds(const TElementContainer& elements) {
			std::set<IdType> ids;
			for (const auto* pElement : elements)
				ids.insert(TTraits::GetId(*pElement));

			return ids;
		}

		template<typename TDelta>
		static void AssertMarkedElements(
				const TDelta& delta,
				const std::set<IdType>& expectedAddedIds,
				const std::set<IdType>& expectedModifiedIds,
				const std::set<IdType>& expectedRemovedIds) {
			EXPECT_EQ(expectedAddedIds, CollectIds(delta.addedElements()));
			EXPECT_EQ(expectedModifiedIds, CollectIds(delta.modifiedElements()));
			EXPECT_EQ(expectedRemovedIds, CollectIds(delta.removedElements()));
		}
	};
}}

#define MAKE_DELTA_ELEMENTS_MIXIN_TEST(TRAITS_NAME, POSTFIX, MODIFICATION_POLICY, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##POSTFIX) { test::DeltaElementsMixinTests<TRAITS_NAME, MODIFICATION_POLICY>::Assert##TEST_NAME(); }

#define DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS_NAME, MODIFICATION_POLICY, POSTFIX) \
	MAKE_DELTA_ELEMENTS_MIXIN_TEST(TRAITS_NAME, POSTFIX, MODIFICATION_POLICY, InitiallyNoElementsAreMarkedAsAddedOrModifiedOrRemoved) \
	MAKE_DELTA_ELEMENTS_MIXIN_TEST(TRAITS_NAME, POSTFIX, MODIFICATION_POLICY, AddedElementsAreMarkedAsAdded) \
	MAKE_DELTA_ELEMENTS_MIXIN_TEST(TRAITS_NAME, POSTFIX, MODIFICATION_POLICY, ModifiedElementsAreMarkedAsModified) \
	MAKE_DELTA_ELEMENTS_MIXIN_TEST(TRAITS_NAME, POSTFIX, MODIFICATION_POLICY, RemovedElementsAreMarkedAsRemoved) \
	MAKE_DELTA_ELEMENTS_MIXIN_TEST(TRAITS_NAME, POSTFIX, MODIFICATION_POLICY, MultipleMarkedElementsCanBeTracked)

#define DEFINE_DELTA_ELEMENTS_MIXIN_TESTS(TRAITS_NAME, POSTFIX) \
	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS_NAME, test::DeltaInsertModificationPolicy, POSTFIX)

