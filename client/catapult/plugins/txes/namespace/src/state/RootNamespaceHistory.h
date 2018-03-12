#pragma once
#include "RootNamespace.h"
#include "catapult/utils/Functional.h"
#include <boost/optional.hpp>
#include <list>
#include <set>

namespace catapult { namespace state {

	/// A root namespace history.
	class RootNamespaceHistory {
	public:
		/// Creates a root namespace history around \a id.
		explicit RootNamespaceHistory(NamespaceId id) : m_id(id)
		{}

		/// Copy constructor.
		/// \note this constructor is needed to ensure that the copy is sharing children among consecutive roots with same owners.
		RootNamespaceHistory(const RootNamespaceHistory& history);

		/// Move constructor.
		RootNamespaceHistory(RootNamespaceHistory&& history) = default;

	public:
		RootNamespace& operator=(const RootNamespace& rhs) = delete;

	public:
		/// Gets a value indicating whether or not the history is empty.
		bool empty() const {
			return m_rootHistory.empty();
		}

		/// Gets the id of the root namespace history.
		NamespaceId id() const {
			return m_id;
		}

		/// Gets the root namespace history size.
		size_t historyDepth() const {
			return m_rootHistory.size();
		}

		/// Gets the number of children of the most recent root namespace.
		size_t numActiveRootChildren() const {
			return m_rootHistory.empty() ? 0 : back().size();
		}

		/// Gets the number of all children.
		/// \note Children are counted more than one time if they are in more than one root namespace.
		size_t numAllHistoricalChildren() const {
			return utils::Sum(m_rootHistory, [](const auto& rootNamespace) { return rootNamespace.size(); });
		}

	public:
		/// Adds a new root namespace around \a owner and \a lifetime at the end of the history.
		void push_back(const Key& owner, const NamespaceLifetime& lifetime);

		/// Removes the last entry in the history.
		void pop_back();

		/// Gets a const reference to the most recent root namespace.
		const RootNamespace& back() const {
			return m_rootHistory.back();
		}

		/// Gets a reference to the most recent root namespace.
		RootNamespace& back() {
			return m_rootHistory.back();
		}

		/// Prunes all root namespaces that are not active at \a height.
		std::set<NamespaceId> prune(Height height);

	public:
		/// Returns a const iterator to the first root namespace.
		auto begin() const {
			return m_rootHistory.cbegin();
		}

		/// Returns a const iterator to the element following the last root namespace.
		auto end() const {
			return m_rootHistory.cend();
		}

	private:
		NamespaceId m_id;
		std::list<RootNamespace> m_rootHistory;
	};
}}
