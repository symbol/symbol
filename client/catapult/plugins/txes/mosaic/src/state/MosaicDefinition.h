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
#include "src/model/MosaicProperties.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Represents a mosaic definition.
	class MosaicDefinition {
	public:
		/// Creates a mosaic definition around \a startHeight, \a ownerAddress, mosaic \a revision and mosaic \a properties.
		MosaicDefinition(Height startHeight, const Address& ownerAddress, uint32_t revision, const model::MosaicProperties& properties)
				: m_startHeight(startHeight)
				, m_ownerAddress(ownerAddress)
				, m_revision(revision)
				, m_properties(properties)
		{}

	public:
		/// Gets the start height.
		Height startHeight() const;

		/// Gets the owner's address.
		const Address& ownerAddress() const;

		/// Gets the revision.
		uint32_t revision() const;

		/// Gets the mosaic properties.
		const model::MosaicProperties& properties() const;

		/// Returns \c true if the mosaic definition has eternal duration.
		bool isEternal() const;

		/// Returns \c true if the mosaic definition is active at \a height.
		bool isActive(Height height) const;

		/// Returns \c true if the mosaic definition is expired at \a height.
		bool isExpired(Height height) const;

	private:
		Height m_startHeight;
		Address m_ownerAddress;
		uint32_t m_revision;
		model::MosaicProperties m_properties;
	};
}}
