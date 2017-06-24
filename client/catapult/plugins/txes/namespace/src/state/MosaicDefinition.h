#pragma once
#include "src/model/MosaicProperties.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Represents a mosaic definition.
	class MosaicDefinition {
	public:
		/// Creates a mosaic definition around \a height, \a owner and mosaic \a properties.
		explicit MosaicDefinition(
				Height height,
				const Key& owner,
				const model::MosaicProperties& properties)
				: m_height(height)
				, m_owner(owner)
				, m_properties(properties)
		{}

	public:
		/// Returns \c true if the mosaic definition has eternal duration.
		bool isEternal() const;

		/// Returns \c true if the mosaic definition is active at \a height.
		bool isActive(Height height) const;

		/// Returns \c true if the mosaic definition is expired at \a height.
		bool isExpired(Height height) const;

		/// Gets the height.
		Height height() const {
			return m_height;
		}

		/// Gets the owner's public key.
		const Key& owner() const {
			return m_owner;
		}

		/// Gets the mosaic properties.
		const model::MosaicProperties& properties() const {
			return m_properties;
		}

	private:
		Height m_height;
		Key m_owner;
		model::MosaicProperties m_properties;
	};
}}
