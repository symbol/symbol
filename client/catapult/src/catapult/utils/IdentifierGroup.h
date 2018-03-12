#pragma once
#include "Hashers.h"
#include <unordered_set>

namespace catapult { namespace utils {

	/// A group of identifiers that share a common (external) attribute.
	template<typename TIdentifier, typename TGroupingKey, typename TIdentifierHasher>
	class IdentifierGroup {
	public:
		/// Unordered set of identifiers.
		using Identifiers = std::unordered_set<TIdentifier, TIdentifierHasher>;

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
