#pragma once
#include "VerifiableEntity.h"
#include "catapult/utils/HexFormatter.h"
#include <iosfwd>
#include <vector>

namespace catapult { namespace model {

	/// Wrapper around a strongly typed entity and its associated metadata.
	template<typename TEntity>
	class WeakEntityInfoT {
	public:
		/// Creates an entity info.
		constexpr WeakEntityInfoT() : m_pEntity(nullptr), m_pHash(nullptr)
		{}

		/// Creates an entity info around \a entity.
		/// \note This is an implicit constructor in order to facilitate calling TransactionPlugin::publish without hashes.
		constexpr WeakEntityInfoT(const TEntity& entity)
				: m_pEntity(&entity)
				, m_pHash(nullptr)
		{}

		/// Creates an entity info around \a entity and \a hash.
		constexpr explicit WeakEntityInfoT(const TEntity& entity, const Hash256& hash)
				: m_pEntity(&entity)
				, m_pHash(&hash)
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

		/// The entity.
		constexpr const TEntity& entity() const {
			return *m_pEntity;
		}

		/// The entity type.
		constexpr model::EntityType type() const {
			return m_pEntity->Type;
		}

		/// The entity hash.
		constexpr const Hash256& hash() const {
			return *m_pHash;
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

	public:
		/// Coerces this info into a differently typed info.
		template<typename TEntityResult>
		WeakEntityInfoT<TEntityResult> cast() const {
			return WeakEntityInfoT<TEntityResult>(static_cast<const TEntityResult&>(entity()), hash());
		}

	private:
		const TEntity* m_pEntity;
		const Hash256* m_pHash;
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

	/// A container of weak entity infos.
	using WeakEntityInfos = std::vector<WeakEntityInfo>;
}}
