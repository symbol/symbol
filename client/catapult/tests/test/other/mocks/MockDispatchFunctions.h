#pragma once
#include "catapult/validators/ParallelValidationPolicy.h"
#include "catapult/validators/SequentialValidationPolicy.h"
#include "catapult/validators/ValidatorTypes.h"
#include "catapult/preprocessor.h"
#include "tests/test/nodeps/ParamsCapture.h"

namespace catapult { namespace mocks {

	/// Parameters captured by a dispatch function.
	struct DispatchParams {
	public:
		/// Creates dispatch params with captured \a entityInfos and \a validationFunctions.
		explicit DispatchParams(
				const model::WeakEntityInfos& entityInfos,
				const validators::ValidationFunctions& validationFunctions)
				: EntityInfos(entityInfos)
				, NumValidationFunctions(validationFunctions.size()) {
			for (const auto& entityInfo : EntityInfos)
				HashCopies.push_back(entityInfo.hash());
		}

	public:
		/// The captured entity infos.
		model::WeakEntityInfos EntityInfos;

		/// The number of captured validation functions.
		size_t NumValidationFunctions;

		/// Copies of the hashes in EntityInfos.
		std::vector<Hash256> HashCopies;
	};

	/// A mock dispatch function that captures the entity infos passed to dispatch.
	template<typename TResult>
	class MockDispatchFunction : public test::ParamsCapture<DispatchParams> {
	public:
		MockDispatchFunction()
				: m_result(validators::ValidationResult::Success)
				, m_numValidateCalls(0)
				, m_validateTrigger(0)
		{}

	public:
		/// Sets the result of dispatch to \a result after \a trigger calls.
		void setResult(validators::ValidationResult result, size_t trigger = 0) {
			m_result = result;
			m_validateTrigger = trigger;
		}

	public:
		/// Invokes the dispatch operation with \a entityInfos and \a validationFunctions.
		auto operator()(
				const model::WeakEntityInfos& entityInfos,
				const validators::ValidationFunctions& validationFunctions) const {
			const_cast<MockDispatchFunction*>(this)->push(entityInfos, validationFunctions);

			// - invoke all sub validators once but ignore their results
			//   (this emulates the real dispatcher delegating to the validationFunctions)
			for (const auto& validationFunction : validationFunctions)
				validationFunction(entityInfos.front());

			// - determine the result based on the call count
			auto result = ++m_numValidateCalls < m_validateTrigger
					? validators::ValidationResult::Success
					: m_result;
			return toResult(result);
		}

	private:
		virtual TResult toResult(validators::ValidationResult result) const = 0;

	private:
		validators::ValidationResult m_result;
		mutable size_t m_numValidateCalls;
		size_t m_validateTrigger;
	};

	/// A mock parallel dispatch function.
	class MockParallelDispatchFunc : public MockDispatchFunction<thread::future<validators::ValidationResult>> {
	private:
		thread::future<validators::ValidationResult> toResult(validators::ValidationResult result) const override {
			thread::promise<validators::ValidationResult> promise;
			promise.set_value(std::move(result));
			return promise.get_future();
		}
	};

	/// Wraps a MockParallelDispatchFunc (\a dispatchFunc) in a ParallelValidationPolicyFunc.
	CATAPULT_INLINE
	validators::ParallelValidationPolicyFunc Wrap(const MockParallelDispatchFunc& dispatchFunc) {
		return validators::ParallelValidationPolicyFunc([&dispatchFunc](
				const auto& entityInfos,
				const auto& validationFunctions) {
			return dispatchFunc(entityInfos, validationFunctions);
		}, std::shared_ptr<void>());
	}

	/// A mock sequential dispatch function.
	class MockSequentialDispatchFunc : public MockDispatchFunction<validators::ValidationResult> {
	private:
		validators::ValidationResult toResult(validators::ValidationResult result) const override {
			return result;
		}
	};

	/// Wraps a MockSequentialDispatchFunc (\a dispatchFunc) in a SequentialValidationPolicyFunc.
	CATAPULT_INLINE
	validators::SequentialValidationPolicyFunc Wrap(const MockSequentialDispatchFunc& dispatchFunc) {
		return validators::SequentialValidationPolicyFunc([&dispatchFunc](
				const auto& entityInfos,
				const auto& validationFunctions) {
			return dispatchFunc(entityInfos, validationFunctions);
		});
	}
}}
