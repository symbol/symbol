#pragma once
#include "catapult/chain/ChainFunctions.h"
#include "catapult/model/WeakEntityInfo.h"
#include "catapult/validators/ValidationResult.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace model {
		struct Transaction;
		class WeakCosignedTransactionInfo;
	}
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace chain {

	/// The result of a partial transaction cosigners validation.
	enum class CosignersValidationResult {
		/// At least one cosigner is missing.
		Missing,

		/// At least one cosigner is ineligible.
		Ineligible,

		/// All cosigners are eligible and sufficient.
		Success,

		/// The transaction failed validation and should be rejected.
		Failure
	};

	/// A validator for validating parts of a partial transaction.
	/// \note Upon completion the full aggregate transaction will be revalidated.
	class PtValidator {
	public:
		virtual ~PtValidator() {}

	public:
		/// A validation result.
		template<typename TNormalizedResult>
		struct Result {
			/// The raw validation result.
			validators::ValidationResult Raw;

			/// The normalized validation result.
			TNormalizedResult Normalized;
		};

	public:
		/// Validates a partial transaction (\a transactionInfo) excluding ineligible and missing cosigners checks.
		virtual Result<bool> validatePartial(const model::WeakEntityInfoT<model::Transaction>& transactionInfo) const = 0;

		/// Validates the cosigners of a partial transaction (\a transactionInfo).
		virtual Result<CosignersValidationResult> validateCosigners(const model::WeakCosignedTransactionInfo& transactionInfo) const = 0;
	};

	/// Creates a default partial transaction validator around \a cache, current time supplier (\a timeSupplier) and \a pluginManager.
	std::unique_ptr<PtValidator> CreatePtValidator(
			const cache::CatapultCache& cache,
			const TimeSupplier& timeSupplier,
			const plugins::PluginManager& pluginManager);
}}
