#pragma once
#include <memory>

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace api {

	/// Base for traits that depend on a transaction registry.
	/// \note This is an implementation detail that is tested indirectly.
	template<typename TEntity>
	struct RegistryDependentTraits {
	public:
		/// Creates traits around \a pRegistry.
		explicit RegistryDependentTraits(const std::shared_ptr<const model::TransactionRegistry>& pRegistry) : m_pRegistry(pRegistry)
		{}

	public:
		/// Returns \c true if \a entity passes size checks.
		bool operator()(const TEntity& entity) {
			return IsSizeValid(entity, *m_pRegistry);
		}

	private:
		std::shared_ptr<const model::TransactionRegistry> m_pRegistry;
	};
}}
