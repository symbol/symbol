/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "RestrictionValueMap.h"
#include "src/model/MosaicRestrictionTypes.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Mosaic restrictions scoped to a mosaic.
	class MosaicGlobalRestriction {
	public:
		/// Single restriction rule.
		struct RestrictionRule {
			/// Identifier of the mosaic providing the restriction key.
			MosaicId ReferenceMosaicId;

			/// Restriction value.
			uint64_t RestrictionValue;

			/// Restriction type.
			model::MosaicRestrictionType RestrictionType;
		};

	public:
		/// Creates a restriction around \a mosaicId.
		explicit MosaicGlobalRestriction(MosaicId mosaicId);

	public:
		/// Gets the mosaic id.
		MosaicId mosaicId() const;

		/// Gets the number of restriction rules.
		size_t size() const;

		/// Gets all restriction keys.
		std::set<uint64_t> keys() const;

	public:
		/// Tries to get the \a rule associated with \a key.
		bool tryGet(uint64_t key, RestrictionRule& rule) const;

		/// Sets the \a rule associated with \a key.
		void set(uint64_t key, const RestrictionRule rule);

	public:
		MosaicId m_mosaicId;
		RestrictionValueMap<RestrictionRule> m_keyRulePairs;
	};
}}
