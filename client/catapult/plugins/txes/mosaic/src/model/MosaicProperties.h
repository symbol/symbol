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
#include "MosaicFlags.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Container for mosaic properties.
	class MosaicProperties {
	public:
		/// Creates zeroed mosaic properties.
		MosaicProperties() : MosaicProperties(MosaicFlags::None, 0, BlockDuration())
		{}

		/// Creates mosaic properties around \a flags, \a divisibility and \a duration.
		MosaicProperties(MosaicFlags flags, uint8_t divisibility, BlockDuration duration)
				: m_flags(flags)
				, m_divisibility(divisibility)
				, m_duration(duration)
		{}

	public:
		/// Gets the mosaic flags.
		MosaicFlags flags() const {
			return m_flags;
		}

		/// Gets the mosaic divisibility.
		uint8_t divisibility() const {
			return m_divisibility;
		}

		/// Gets the mosaic duration.
		BlockDuration duration() const {
			return m_duration;
		}

		/// Returns \c true if mosaic flags contain \a testedFlag.
		bool is(MosaicFlags testedFlag) const {
			return HasFlag(testedFlag, m_flags);
		}

	public:
		/// Returns \c true if this properties bag is equal to \a rhs.
		bool operator==(const MosaicProperties& rhs) const {
			return m_flags == rhs.m_flags
					&& m_divisibility == rhs.m_divisibility
					&& m_duration == rhs.m_duration;
		}

		/// Returns \c true if this properties bag is not equal to \a rhs.
		bool operator!=(const MosaicProperties& rhs) const {
			return !(*this == rhs);
		}

	private:
		MosaicFlags m_flags;
		uint8_t m_divisibility;
		BlockDuration m_duration;
	};
}}
