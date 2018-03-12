#pragma once
#include <memory>

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace api {

	/// Base for traits that depend on a transaction registry.
	/// \note This is an implementation detail that is tested indirectly.
	template<typename TEntity>
	struct RegistryDependentTraits {
	public:
		/// Creates traits around \a registry.
		explicit RegistryDependentTraits(const model::TransactionRegistry& registry) : m_registry(registry)
		{}

	public:
		/// Returns \c true if \a entity passes size checks.
		bool operator()(const TEntity& entity) {
			return IsSizeValid(entity, m_registry);
		}

	private:
		const model::TransactionRegistry& m_registry;
	};
}}
