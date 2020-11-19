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
#include <unordered_set>

namespace catapult { namespace utils {

	/// Group of identifiers that share a common (external) attribute.
	template<typename TIdentifier, typename TGroupingKey, typename TIdentifierHasher>
	class IdentifierGroup {
	public:
		/// Unordered set of identifiers.
		using Identifiers = std::unordered_set<TIdentifier, TIdentifierHasher>;

		/// Type of grouping key.
		using GroupingKeyType = TGroupingKey;

	public:
		/// Creates a group around a given \a key.
		explicit IdentifierGroup(const TGroupingKey& key) : m_key(key)
		{}

	public:
		/// Gets the grouping key.
		const TGroupingKey& key() const {
			return m_key;
		}

		/// Gets the identifiers.
		const Identifiers& identifiers() const {
			return m_identifiers;
		}

	public:
		/// Returns \c true if the group is empty, \c false otherwise.
		bool empty() const {
			return m_identifiers.empty();
		}

		/// Gets the number of identifiers in the group.
		size_t size() const {
			return m_identifiers.size();
		}

	public:
		/// Adds \a identifier to the group.
		void add(const TIdentifier& identifier) {
			m_identifiers.insert(identifier);
		}

		/// Removes \a identifier from the group.
		void remove(const TIdentifier& identifier) {
			m_identifiers.erase(identifier);
		}

	private:
		TGroupingKey m_key;
		Identifiers m_identifiers;
	};
}}
