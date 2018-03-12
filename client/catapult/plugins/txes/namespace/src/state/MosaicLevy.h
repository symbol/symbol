#pragma once
#include "catapult/model/Mosaic.h"
#include "catapult/types.h"
#include <functional>
#include <vector>

namespace catapult { namespace state {

	/// Prototype for a mosaic levy rule.
	using MosaicLevyRule = std::function<model::Mosaic (const model::Mosaic&)>;

	/// A mosaic levy.
	class MosaicLevy {
	public:
		/// Creates a mosaic levy around \a id, \a recipient and levy \a rules.
		explicit MosaicLevy(MosaicId id, const Address& recipient, const std::vector<MosaicLevyRule>& rules)
				: m_id(id)
				, m_recipient(recipient)
				, m_rules(rules)
		{}

	public:
		/// Gets the mosaic id.
		MosaicId id() const {
			return m_id;
		}

		/// Gets the recipient.
		const Address& recipient() const {
			return m_recipient;
		}

		/// Gets the rules.
		const std::vector<MosaicLevyRule>& rules() const {
			return m_rules;
		}

	private:
		MosaicId m_id;
		Address m_recipient;
		std::vector<MosaicLevyRule> m_rules;
	};
}}
