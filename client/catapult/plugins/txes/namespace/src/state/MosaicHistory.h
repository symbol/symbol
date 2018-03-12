#pragma once
#include "MosaicEntry.h"
#include "src/model/NamespaceConstants.h"
#include "catapult/preprocessor.h"
#include "catapult/types.h"
#include <list>

namespace catapult { namespace state {

	/// A mosaic history.
	class MosaicHistory {
	public:
		/// Creates a mosaic history around \a namespaceId and \a id.
		MosaicHistory(NamespaceId namespaceId, MosaicId id) : m_namespaceId(namespaceId), m_id(id)
		{}

	public:
		/// Gets a value indicating whether or not the history is empty.
		bool empty() const {
			return m_history.empty();
		}

		/// Gets the namespace id of the mosaic history.
		NamespaceId namespaceId() const {
			return m_namespaceId;
		}

		/// Gets the id of the mosaic history.
		MosaicId id() const {
			return m_id;
		}

		/// Gets the mosaic history size.
		size_t historyDepth() const {
			return m_history.size();
		}

	public:
		/// Adds a new mosaic entry composed of \a definition and \a supply at the end of the history.
		void push_back(const MosaicDefinition& definition, Amount supply);

		/// Removes the last entry in the history.
		void pop_back();

		/// Gets a const reference to the most recent mosaic entry.
		const MosaicEntry& back() const {
			return m_history.back();
		}

		/// Gets a reference to the most recent mosaic entry.
		MosaicEntry& back() {
			return m_history.back();
		}

		/// Prunes all mosaic entries that are not active at \a height.
		void prune(Height height);

	public:
		/// Returns a const iterator to the first entry.
		auto begin() const {
			return m_history.cbegin();
		}

		/// Returns a const iterator to the element following the last entry.
		auto end() const {
			return m_history.cend();
		}

	public:
		/// Returns \c true if history is active at \a height.
		bool isActive(Height height) const {
			return !empty() && back().definition().isActive(height);
		}

	private:
		NamespaceId m_namespaceId;
		MosaicId m_id;
		std::list<MosaicEntry> m_history;
	};
}}
