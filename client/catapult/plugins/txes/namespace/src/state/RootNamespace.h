#pragma once
#include "Namespace.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult { namespace state {

	/// A root namespace.
	class RootNamespace {
	public:
		using Children = std::unordered_map<NamespaceId, Namespace::Path, utils::BaseValueHasher<NamespaceId>>;

	public:
		/// Creates a root namespace around \a id, \a owner and \a liftime.
		explicit RootNamespace(NamespaceId id, const Key& owner, const NamespaceLifetime& lifetime)
				: RootNamespace(id, owner, lifetime, std::make_shared<Children>())
		{}

		/// Copy constructor.
		RootNamespace(const RootNamespace& root)
				: RootNamespace(root.m_id, root.m_owner, root.m_lifetime, std::make_shared<Children>(root.children()))
		{}

		/// Move constructor.
		RootNamespace(RootNamespace&& root) = default;

	public:
		RootNamespace& operator=(const RootNamespace& rhs) = delete;

	private:
		explicit RootNamespace(
				NamespaceId id,
				const Key& owner,
				const NamespaceLifetime& lifetime,
				const std::shared_ptr<Children>& pChildren)
				: m_id(id)
				, m_owner(owner)
				, m_lifetime(lifetime)
				, m_pChildren(pChildren)
		{}

	public:
		/// Gets the namespace id.
		NamespaceId id() const {
			return m_id;
		}

		/// Gets a const reference to the children.
		const Children& children() const {
			return *m_pChildren;
		}

		/// Gets a const reference to the owner of this namespace.
		const Key& owner() const {
			return m_owner;
		}

		/// Gets a const reference to the lifetime of this namespace.
		const NamespaceLifetime& lifetime() const {
			return m_lifetime;
		}

		/// Returns \c true if this root namespace has no children.
		bool empty() const {
			return m_pChildren->empty();
		}

		/// Gets the number of child namespaces.
		size_t size() const {
			return m_pChildren->size();
		}

	public:
		/// Gets a child namespace specified by its namespace \a id.
		/// \note This method throws if the id is unknown.
		Namespace child(NamespaceId id) const;

		/// Adds the child namespace \a ns.
		void add(const Namespace& ns);

		/// Removes a child namespace specified by \a id.
		/// \note This method throws if the id is unknown.
		void remove(NamespaceId id);

	public:
		/// Returns \c true if this root namespace is equal to \a rhs.
		bool operator==(const RootNamespace& rhs) const;

		/// Returns \c true if this root namespace is not equal to \a rhs.
		bool operator!=(const RootNamespace& rhs) const {
			return !(*this == rhs);
		}

	public:
		/// Creates a new root namespace with \a lifetime.
		/// \note The method shares the children of this root namespace with the new root namespace.
		RootNamespace renew(const NamespaceLifetime& newLifetime) const {
			return RootNamespace(m_id, m_owner, newLifetime, m_pChildren);
		}

	private:
		NamespaceId m_id;
		Key m_owner;
		NamespaceLifetime m_lifetime;
		std::shared_ptr<Children> m_pChildren;
	};
}}
