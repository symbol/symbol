#pragma once
#include "MosaicLevy.h"
#include <functional>
#include <vector>

namespace catapult { namespace model { struct Mosaic; } }

namespace catapult { namespace state {

	/// The available mosaic levy rule ids.
	enum class RuleId : uint8_t {
		/// Use lower bound fee.
		Constant,
		/// Use percentile fee.
		Percentile,
		/// Use the max(lower bound fee, percentile fee).
		Lower_Bounded_Percentile,
		/// Use min(upper bound fee, percentile fee).
		Upper_Bounded_Percentile,
		/// Use min(upper bound fee, max(lower bound fee, percentile fee)).
		Bounded_Percentile
	};

	/// A factory for creating mosaic levy rules.
	class MosaicLevyRuleFactory {
	public:
		/// Prototype for a mosaic levy rule creator.
		using MosaicLevyRuleCreator = std::function<MosaicLevyRule (Amount lowerBount, Amount upperBound, uint64_t percentile)>;

	public:
		/// Creates a mosaic levy rule factory.
		MosaicLevyRuleFactory();

	public:
		/// Gets the number of rule creation functions in the factory.
		size_t size() const {
			return m_rules.size();
		}

	public:
		/// Creates a mosaic levy rule around \a lowerBound, \a upperBound, \a percentile and \a ruleId.
		MosaicLevyRule createRule(Amount lowerBound, Amount upperBound, uint64_t percentile, RuleId ruleId);

	private:
		std::vector<MosaicLevyRuleCreator> m_rules;
	};
}}
