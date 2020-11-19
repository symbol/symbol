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
#include "VerifiableEntity.h"
#include "catapult/utils/HexFormatter.h"
#include <iosfwd>
#include <vector>

namespace catapult { namespace model { struct BlockHeader; } }

namespace catapult { namespace model {

	/// Wrapper around a strongly typed entity and its associated metadata.
	template<typename TEntity>
	class WeakEntityInfoT {
	public:
		/// Creates an entity info.
		constexpr WeakEntityInfoT()
				: m_pEntity(nullptr)
				, m_pHash(nullptr)
				, m_pAssociatedBlockHeader(nullptr)
		{}

		/// Creates an entity info around \a entity.
		/// \note This is an implicit constructor in order to facilitate calling TransactionPlugin::publish without hashes.
		constexpr WeakEntityInfoT(const TEntity& entity)
				: m_pEntity(&entity)
				, m_pHash(nullptr)
				, m_pAssociatedBlockHeader(nullptr)
		{}

		/// Creates an entity info around \a entity and \a hash.
		constexpr WeakEntityInfoT(const TEntity& entity, const Hash256& hash)
				: m_pEntity(&entity)
				, m_pHash(&hash)
				, m_pAssociatedBlockHeader(nullptr)
		{}

		/// Creates an entity info around \a entity, \a hash and \a associatedBlockHeader.
		constexpr WeakEntityInfoT(const TEntity& entity, const Hash256& hash, const BlockHeader& associatedBlockHeader)
				: m_pEntity(&entity)
				, m_pHash(&hash)
				, m_pAssociatedBlockHeader(&associatedBlockHeader)
		{}

	public:
		/// Returns \c true if this info has an associated entity.
		constexpr bool isSet() const {
			return !!m_pEntity;
		}

		/// Returns \c true if this info has an associated hash.
		constexpr bool isHashSet() const {
			return !!m_pHash;
		}

		/// Returns \c true if this info has an associated block header.
		constexpr bool isAssociatedBlockHeaderSet() const {
			return !!m_pAssociatedBlockHeader;
		}

	public:
		/// Gets the entity.
		constexpr const TEntity& entity() const {
			return *m_pEntity;
		}

		/// Gets the entity type.
		constexpr EntityType type() const {
			return m_pEntity->Type;
		}

		/// Gets the entity hash.
		constexpr const Hash256& hash() const {
			return *m_pHash;
		}

		/// Gets the associated block header.
		constexpr const BlockHeader& associatedBlockHeader() const {
			return *m_pAssociatedBlockHeader;
		}

	public:
		/// Coerces this info into a differently typed info.
		template<typename TEntityResult>
		WeakEntityInfoT<TEntityResult> cast() const {
			const auto& typedEntity = static_cast<const TEntityResult&>(entity());
			return isAssociatedBlockHeaderSet()
					? WeakEntityInfoT<TEntityResult>(typedEntity, hash(), associatedBlockHeader())
					: WeakEntityInfoT<TEntityResult>(typedEntity, hash());
		}

	public:
		/// Returns \c true if this info is equal to \a rhs.
		constexpr bool operator==(const WeakEntityInfoT& rhs) const {
			return m_pEntity == rhs.m_pEntity && m_pHash == rhs.m_pHash;
		}

		/// Returns \c true if this info is not equal to \a rhs.
		constexpr bool operator!=(const WeakEntityInfoT& rhs) const {
			return !(*this == rhs);
		}

	private:
		const TEntity* m_pEntity;
		const Hash256* m_pHash;
		const BlockHeader* m_pAssociatedBlockHeader;
	};

	using WeakEntityInfo = WeakEntityInfoT<VerifiableEntity>;

	/// Insertion operator for outputting \a entityInfo to \a out.
	template<typename TEntity>
	std::ostream& operator<<(std::ostream& out, const WeakEntityInfoT<TEntity>& entityInfo) {
		if (!entityInfo.isSet()) {
			out << "WeakEntityInfo (unset)";
		} else {
			const auto& hash = entityInfo.hash();
			out << entityInfo.entity() << " [" << utils::HexFormat(hash.cbegin(), hash.cend()) << "]";
		}

		return out;
	}

	/// Container of weak entity infos.
	using WeakEntityInfos = std::vector<WeakEntityInfo>;
}}
