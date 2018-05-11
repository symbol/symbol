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
		Amount supply() const {
			return m_supply;
		}

	public:
		/// Increases the supply by \a delta.
		void increaseSupply(Amount delta) {
			m_supply = m_supply + delta;
		}

		// Decreases the supply by \a delta.
		void decreaseSupply(Amount delta) {
			if (delta > m_supply)
				CATAPULT_THROW_INVALID_ARGUMENT_2("cannot decrease mosaic supply below zero (supply, delta)", m_supply, delta);

			m_supply = m_supply - delta;
		}

	private:
		Amount m_supply;
	};

	// endregion

	// region MosaicEntryLevyMixin

	/// Mixin for storing and modifying a levy in MosaicEntry.
	class MosaicEntryLevyMixin {
	public:
		/// Returns \c true if the mosaic has a levy.
		bool hasLevy() const {
			return !!m_pLevy;
		}

		/// Gets the mosaic levy.
		const MosaicLevy& levy() const {
			if (!hasLevy())
				CATAPULT_THROW_RUNTIME_ERROR("mosaic has no levy");

			return *m_pLevy;
		}

	public:
		/// Sets the mosaic levy to \a pLevy.
		void setLevy(std::shared_ptr<MosaicLevy>&& pLevy) {
			if (hasLevy())
				CATAPULT_THROW_RUNTIME_ERROR("mosaic already has levy set");

			m_pLevy = std::move(pLevy);
		}

	private:
		std::shared_ptr<MosaicLevy> m_pLevy;
	};

	// endregion

	/// A tuple composed of a mosaic definition and its current state.
	/// \note The mosaic entry has no mosaic levy by default.
	class MosaicEntry : public MosaicEntrySupplyMixin, public MosaicEntryLevyMixin {
	public:
		/// Creates a mosaic entry around an owning \a namespaceId, a mosaic \a id and a mosaic \a definition.
		MosaicEntry(NamespaceId namespaceId, MosaicId id, const MosaicDefinition& definition)
				: m_namespaceId(namespaceId)
				, m_id(id)
				, m_definition(definition)
		{}

	public:
		/// Gets the owning namespace id.
		NamespaceId namespaceId() const {
			return m_namespaceId;
		}

		/// Gets the mosaic id.
		MosaicId mosaicId() const {
			return m_id;
		}

		/// Gets the mosaic definition.
		const MosaicDefinition& definition() const {
			return m_definition;
		}

	private:
		NamespaceId m_namespaceId;
		MosaicId m_id;
		MosaicDefinition m_definition;
	};
}}
