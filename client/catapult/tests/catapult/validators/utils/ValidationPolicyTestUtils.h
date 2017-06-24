#pragma once
#include "MockEntityValidator.h"
#include "catapult/model/Block.h"
#include "catapult/validators/AggregateEntityValidator.h"

namespace catapult { namespace test {

	/// Wraps a vector of entity infos and their backing memory.
	class EntityInfoContainerWrapper {
	public:
		/// Creates a wrapper around \a count entity infos.
		explicit EntityInfoContainerWrapper(size_t count);

	public:
		/// Returns a vector containing all entity infos.
		model::WeakEntityInfos toVector() const;

	private:
		std::shared_ptr<model::Block> m_pBlock;
		model::BasicContiguousEntityContainer<model::Transaction> m_container;
		std::vector<Hash256> m_hashes;
	};

	/// Creates a wrapper around \a count entity infos such that the entity infos have incrementing timestamps.
	EntityInfoContainerWrapper CreateEntityInfos(size_t count);

	/// Returns the value of the counter at \a index in \a states.
	template<typename TStates>
	size_t CounterAt(const TStates& states, size_t index) {
		return states[index]->counter();
	}

	/// Creates \a count counter states.
	template<typename TStates>
	TStates CreateCounterStates(size_t count) {
		TStates states;
		for (auto i = 0u; i < count; ++i)
			states.push_back(std::make_shared<typename TStates::value_type::element_type>());

		return states;
	}

	/// Adds validators created from \a pairs and \a states to \a builder.
	template<typename TPairs, typename TStates>
	void AddValidators(
			validators::ValidatorVectorT<const validators::ValidatorContext&>& builder,
			const TPairs& pairs,
			const TStates& states) {
		using MockValidatorType = mocks::MockEntityValidator<typename TStates::value_type::element_type>;

		size_t i = 0;
		for (auto pair : pairs) {
			auto pValidator = std::make_unique<MockValidatorType>(pair.second, states[i++], pair.first);
			builder.push_back(std::move(pValidator));
		}
	}

	/// Creates an aggregate validator from \a builder.
	CATAPULT_INLINE
	auto CreateAggregateValidator(validators::ValidatorVectorT<const validators::ValidatorContext&>&& builder) {
		return std::make_unique<validators::stateful::AggregateEntityValidator>(std::move(builder));
	}

	/// Creates an aggregate validator from \a pairs and \a states.
	template<typename TPairs, typename TStates>
	auto CreateValidator(const TPairs& pairs, const TStates& states) {
		validators::ValidatorVectorT<const validators::ValidatorContext&> builder;
		AddValidators(builder, pairs, states);
		return CreateAggregateValidator(std::move(builder));
	}

	/// Gets the number of sub validators in the aggregate validator (\a validator).
	template<typename TValidator>
	size_t GetNumSubValidators(const TValidator& validator) {
		return validator.names().size();
	}
}}
