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
#include "BaseSetCommitPolicy.h"
#include "BaseSetDefaultTraits.h"
#include "BaseSetFindIterator.h"
#include <memory>

namespace catapult {
	namespace deltaset {
		template<typename TElementTraits, typename TSetTraits>
		class BaseSetDelta;

		template<typename TSetTraits>
		class BaseSetIterationView;
	}
}

namespace catapult { namespace deltaset {

	/// Base set.
	/// \tparam TElementTraits Traits describing the type of element.
	/// \tparam TSetTraits Traits describing the underlying set.
	/// \tparam TCommitPolicy The policy for committing changes to a base set.
	///
	/// \note: 1) this class is not thread safe.
	///        2) if TSetTraits::SetType is an unordered set, the element must implement operator ==
	///        3) if MutableTypeTraits are used, the element must implement a (deep) copy
	template<
			typename TElementTraits,
			typename TSetTraits,
			typename TCommitPolicy = BaseSetCommitPolicy<TSetTraits>
	>
	class BaseSet : public utils::MoveOnly {
	public:
		using ElementType = typename TElementTraits::ElementType;
		using SetType = typename TSetTraits::SetType;
		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = FindTraitsT<ElementType, TSetTraits::AllowsNativeValueModification>;
		using DeltaType = BaseSetDelta<TElementTraits, TSetTraits>;

		using FindConstIterator = BaseSetFindIterator<FindTraits, TSetTraits>;

	public:
		/// Creates a base set.
		/// \a args are forwarded to the underlying container.
		template<typename... TArgs>
		explicit BaseSet(TArgs&&... args) : m_elements(std::forward<TArgs>(args)...)
		{}

	public:
		/// Gets a value indicating whether or not the set is empty.
		bool empty() const {
			return m_elements.empty();
		}

		/// Gets the size of this set.
		size_t size() const {
			return m_elements.size();
		}

		/// Searches for \a key in this set.
		/// Gets a pointer to the matching element if it is found or \c nullptr if it is not found.
		FindConstIterator find(const KeyType& key) const {
			auto iter = m_elements.find(key);
			return m_elements.cend() != iter ? FindConstIterator(std::move(iter)) : FindConstIterator();
		}

		/// Searches for \a key in this set.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const KeyType& key) const {
			return m_elements.cend() != m_elements.find(key);
		}

	public:
		/// Gets a delta based on the same original elements as this set.
		std::shared_ptr<DeltaType> rebase() {
			if (m_pWeakDelta.lock())
				CATAPULT_THROW_RUNTIME_ERROR("only a single attached delta is allowed at a time");

			auto pDelta = std::make_shared<DeltaType>(m_elements);
			m_pWeakDelta = pDelta;
			return pDelta;
		}

		/// Gets a delta based on the same original elements as this set
		/// but without the ability to commit any changes to the original set.
		std::shared_ptr<DeltaType> rebaseDetached() const {
			return std::make_shared<DeltaType>(m_elements);
		}

	public:
		/// Commits all changes in the rebased cache.
		/// \a args are forwarded to the commit policy.
		template<typename... TArgs>
		void commit(TArgs&&... args) {
			auto pDelta = m_pWeakDelta.lock();
			if (!pDelta)
				CATAPULT_THROW_RUNTIME_ERROR("attempting to commit changes to a set without any outstanding attached deltas");

			auto deltas = pDelta->deltas();
			TCommitPolicy::Update(m_elements, deltas, std::forward<TArgs>(args)...);
			pDelta->reset();
		}

	private:
		SetType m_elements;
		std::weak_ptr<DeltaType> m_pWeakDelta;

	private:
		template<typename TElementTraits2, typename TSetTraits2, typename TCommitPolicy2>
		friend bool IsBaseSetIterable(const BaseSet<TElementTraits2, TSetTraits2, TCommitPolicy2>& set);

		template<typename TElementTraits2, typename TSetTraits2, typename TCommitPolicy2>
		friend BaseSetIterationView<TSetTraits2> MakeIterableView(const BaseSet<TElementTraits2, TSetTraits2, TCommitPolicy2>& set);
	};
}}
