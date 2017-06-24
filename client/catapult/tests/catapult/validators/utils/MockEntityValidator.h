#pragma once
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/validators/ValidatorTypes.h"
#include "tests/test/plugins/ValidatorTestUtils.h"

namespace catapult { namespace mocks {

	/// Single threaded mock validator state.
	struct SingleThreadedValidatorState {
	public:
		SingleThreadedValidatorState() : m_counter(0)
		{}

	public:
		size_t counter() const {
			return m_counter;
		}

	public:
		void increment(const model::VerifiableEntity&) {
			++m_counter;
		}

	private:
		size_t m_counter;
	};

	/// Represents an empty validator context
	struct EmptyValidatorContext {
	public:
		/// Creates an empty validator context.
		EmptyValidatorContext()
				: m_cache({})
				, m_cacheView(m_cache.createView())
				, m_readOnlyCache(m_cacheView.toReadOnly())
				, m_context(test::CreateValidatorContext(Height(1234), m_readOnlyCache))
		{}

	public:
		/// Returns a constant reference to the validator context.
		auto cref() const {
			return std::cref(m_context);
		}

	private:
		cache::CatapultCache m_cache;
		cache::CatapultCacheView m_cacheView;
		cache::ReadOnlyCatapultCache m_readOnlyCache;
		validators::ValidatorContext m_context;
	};

	/// A mock entity validator.
	template<typename TState = SingleThreadedValidatorState>
	class MockEntityValidator : public validators::stateful::EntityValidator {
	public:
		/// Creates a mock validator with \a name around \a pState that returns \a result from validate.
		MockEntityValidator(
				const validators::ValidationResult& result,
				const std::shared_ptr<TState>& pState,
				const std::string& name)
				: m_result(result)
				, m_pState(pState)
				, m_name(name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		validators::ValidationResult validate(
				const model::WeakEntityInfo& entityInfo,
				const validators::ValidatorContext&) const override {
			m_pState->increment(entityInfo.entity());
			return m_result;
		}

	private:
		validators::ValidationResult m_result;
		std::shared_ptr<TState> m_pState;
		std::string m_name;
	};
}}
