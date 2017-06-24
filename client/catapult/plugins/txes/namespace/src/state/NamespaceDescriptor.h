#pragma once
#include "Namespace.h"
#include "src/constants.h"

namespace catapult { namespace state { class RootNamespace; } }

namespace catapult { namespace state {

	/// A namespace descriptor.
	struct NamespaceDescriptor {
	public:
		/// Creates a namespace descriptor around \a path, \a pRootNamespace, \a index and \a isActive.
		explicit NamespaceDescriptor(
				const Namespace::Path& path,
				const std::shared_ptr<const state::RootNamespace>& pRootNamespace,
				uint32_t index,
				bool isActive)
				: Path(path)
				, pRoot(pRootNamespace)
				, Index(index)
				, IsActive(isActive)
		{}

	public:
		/// Returns \c true if the described namespace is a root namespace.
		bool IsRoot() const {
			return 1 == Path.size();
		}

	public:
		/// The namespace path.
		const Namespace::Path Path;

		/// The associated root namespace.
		const std::shared_ptr<const state::RootNamespace> pRoot;

		/// The index in the root namespace history.
		uint32_t Index;

		/// Flag indicating whether or not the namespace is active.
		bool IsActive;
	};
}}
