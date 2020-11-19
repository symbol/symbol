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
#include "BaseSet.h"

namespace catapult { namespace deltaset {

	/// View that provides iteration support to a base set.
	template<typename TSetTraits>
	class BaseSetIterationView {
	private:
		using SetType = typename TSetTraits::MemorySetType;
		using KeyType = typename TSetTraits::KeyType;

	public:
		/// Creates a view around \a set.
		explicit BaseSetIterationView(const SetType& set) : m_set(set)
		{}

	public:
		/// Gets an iterator that points to the element with \a key if it is contained in this set, or end() otherwise.
		auto findIterator(const KeyType& key) const {
			return m_set.find(key);
		}

	public:
		/// Gets a const iterator to the first element of the underlying set.
		auto begin() const {
			return m_set.cbegin();
		}

		/// Gets a const iterator to the element following the last element of the underlying set.
		auto end() const {
			return m_set.cend();
		}

	private:
		const SetType& m_set;
	};

	/// Returns \c true if \a set is iterable.
	template<typename TSet>
	bool IsSetIterable(const TSet&) {
		return true;
	}

	/// Selects the iterable set from \a set.
	template<typename TSet>
	const TSet& SelectIterableSet(const TSet& set) {
		return set;
	}

	/// Returns \c true if \a set is iterable.
	template<typename TElementTraits, typename TSetTraits, typename TCommitPolicy>
	bool IsBaseSetIterable(const BaseSet<TElementTraits, TSetTraits, TCommitPolicy>& set) {
		return IsSetIterable(set.m_elements);
	}

	/// Makes a base \a set iterable.
	/// \note This should only be supported for in memory views.
	template<typename TElementTraits, typename TSetTraits, typename TCommitPolicy>
	BaseSetIterationView<TSetTraits> MakeIterableView(const BaseSet<TElementTraits, TSetTraits, TCommitPolicy>& set) {
		return BaseSetIterationView<TSetTraits>(SelectIterableSet(set.m_elements));
	}
}}
