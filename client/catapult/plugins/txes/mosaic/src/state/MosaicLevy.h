/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
