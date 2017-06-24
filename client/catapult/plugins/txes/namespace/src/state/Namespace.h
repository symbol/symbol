#pragma once
#include "NamespaceLifetime.h"
#include "src/model/NamespaceConstants.h"
#include "catapult/utils/Array.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// A catapult namespace.
	class Namespace {
	public:
		using Path = utils::Array<NamespaceId, Namespace_Max_Depth>;

	public:
		/// Creates a namespace around \a path.
		explicit Namespace(const Path& path) : m_path(path) {
			if (m_path.empty())
				CATAPULT_THROW_OUT_OF_RANGE("path cannot be empty");
		}

	public:
		/// Gets the namespace id.
		NamespaceId id() const {
			return m_path[m_path.size() - 1];
		}

		/// Gets the parent namespace id.
		NamespaceId parentId() const {
			return 1 == m_path.size() ? Namespace_Base_Id : m_path[m_path.size() - 2];
		}

		/// Gets the corresponding root namespace id.
		NamespaceId rootId() const {
			return m_path[0];
		}

		/// Gets a value indicating whether or not this namespace is a root namespace.
		bool isRoot() const {
			return 1 == m_path.size();
		}

		/// Gets the path.
		const Path& path() const {
			return m_path;
		}

	public:
		/// Creates a child namespace of this namespace with namespace identifier \a id.
		Namespace createChild(NamespaceId id) const {
			if (Namespace_Max_Depth == m_path.size())
				CATAPULT_THROW_OUT_OF_RANGE("maximum depth for namepsace path exceeded");

			Path childPath = m_path;
			childPath.push_back(id);
			return Namespace(childPath);
		}

	public:
		/// Returns \c true if this namespace is equal to \a rhs.
		bool operator==(const Namespace& rhs) const {
			return m_path == rhs.m_path;
		}

		/// Returns \c true if this namespace is not equal to \a rhs.
		bool operator!=(const Namespace& rhs) const {
			return !(*this == rhs);
		}

	private:
		Path m_path;
	};
}}
