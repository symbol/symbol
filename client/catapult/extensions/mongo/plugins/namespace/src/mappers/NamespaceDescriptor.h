#pragma once
#include "plugins/txes/namespace/src/state/RootNamespace.h"

namespace catapult { namespace mongo { namespace plugins {

	/// A namespace descriptor.
	struct NamespaceDescriptor {
	public:
		/// Creates a namespace descriptor around \a path, \a pRootNamespace, \a ownerAddress, \a index and \a isActive.
		explicit NamespaceDescriptor(
				const state::Namespace::Path& path,
				const std::shared_ptr<const state::RootNamespace>& pRootNamespace,
				const Address& ownerAddress,
				uint32_t index,
				bool isActive)
				: Path(path)
				, pRoot(pRootNamespace)
				, OwnerAddress(ownerAddress)
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
		const state::Namespace::Path Path;

		/// The associated root namespace.
		const std::shared_ptr<const state::RootNamespace> pRoot;

		/// The owner address.
		/// \note Address is stored to minimize number of conversions.
		Address OwnerAddress;

		/// The index in the root namespace history.
		uint32_t Index;

		/// Flag indicating whether or not the namespace is active.
		bool IsActive;
	};
}}}
