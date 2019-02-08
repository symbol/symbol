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
#include "MosaicDefinition.h"

namespace catapult { namespace state { class MosaicLevy; } }

namespace catapult { namespace state {

	// region MosaicEntrySupplyMixin

	/// Mixin for storing and modifying a supply in MosaicEntry.
	class MosaicEntrySupplyMixin {
	public:
		/// Gets the mosaic supply.
		Amount supply() const;

	public:
		/// Increases the supply by \a delta.
		void increaseSupply(Amount delta);

		// Decreases the supply by \a delta.
		void decreaseSupply(Amount delta);

	private:
		Amount m_supply;
	};

	// endregion

	// region MosaicEntryLevyMixin

	/// Mixin for storing and modifying a levy in MosaicEntry.
	class MosaicEntryLevyMixin {
	public:
		/// Returns \c true if the mosaic has a levy.
		bool hasLevy() const;

		/// Gets the mosaic levy.
		const MosaicLevy& levy() const;

	public:
		/// Sets the mosaic levy to \a pLevy.
		void setLevy(std::shared_ptr<MosaicLevy>&& pLevy);

	private:
		std::shared_ptr<MosaicLevy> m_pLevy;
	};

	// endregion

	// region MosaicEntry

	/// A tuple composed of a mosaic definition and its current state.
	/// \note The mosaic entry has no mosaic levy by default.
	class MosaicEntry : public MosaicEntrySupplyMixin, public MosaicEntryLevyMixin {
	public:
		/// Creates a mosaic entry around mosaic \a id and mosaic \a definition.
		MosaicEntry(MosaicId id, const MosaicDefinition& definition);

	public:
		/// Gets the mosaic id.
		MosaicId mosaicId() const;

		/// Gets the mosaic definition.
		const MosaicDefinition& definition() const;

		/// Returns \c true if entry is active at \a height.
		bool isActive(Height height) const;

	private:
		MosaicId m_id;
		MosaicDefinition m_definition;
	};

	// endregion
}}
