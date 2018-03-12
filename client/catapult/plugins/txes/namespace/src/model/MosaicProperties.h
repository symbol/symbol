#pragma once
#include "MosaicProperty.h"
#include "NamespaceConstants.h"
#include "catapult/utils/Casting.h"
#include <array>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Mosaic properties header.
	struct MosaicPropertiesHeader {
		/// The number of elements in optional properties array.
		uint8_t Count;

		/// Mosaic flags.
		MosaicFlags Flags;

		/// Mosaic divisibility.
		uint8_t Divisibility;
	};

#pragma pack(pop)

	/// Container for mosaic properties.
	class MosaicProperties {
	private:
		using PropertiesContainer = std::array<MosaicProperty, Num_Mosaic_Properties>;

	public:
		/// Type of values container.
		using PropertyValuesContainer = std::array<uint64_t, Num_Mosaic_Properties>;

	private:
		explicit MosaicProperties(const PropertyValuesContainer& values) {
			uint8_t i = 0;
			for (auto value : values) {
				m_properties[i] = { static_cast<MosaicPropertyId>(i), value };
				++i;
			}
		}

	public:
		/// Creates mosaic properties from \a values.
		static MosaicProperties FromValues(const PropertyValuesContainer& values) {
			return MosaicProperties(values);
		}

	public:
		/// Gets the number of properties.
		size_t size() const {
			return Num_Mosaic_Properties;
		}

		/// Returns a const iterator to the first property.
		auto begin() const {
			return m_properties.cbegin();
		}

		/// Returns a const iterator to the element following the last property.
		auto end() const {
			return m_properties.cend();
		}

	private:
		template<typename T>
		T property(MosaicPropertyId id) const {
			return T(m_properties[utils::to_underlying_type(id)].Value);
		}

	public:
		/// Returns \c true if mosaic flags contain \a testedFlag.
		bool is(MosaicFlags testedFlag) const {
			return HasFlag(testedFlag, property<MosaicFlags>(MosaicPropertyId::Flags));
		}

		/// Gets mosaic divisibility.
		uint8_t divisibility() const {
			return property<uint8_t>(MosaicPropertyId::Divisibility);
		}

		/// Gets mosaic duration.
		BlockDuration duration() const {
			return property<BlockDuration>(MosaicPropertyId::Duration);
		}

	public:
		/// Returns \c true if this properties bag is equal to \a rhs.
		bool operator==(const MosaicProperties& rhs) const {
			for (auto i = 0u; i < Num_Mosaic_Properties; ++i) {
				if (m_properties[i].Value != rhs.m_properties[i].Value)
					return false;
			}

			return true;
		}

		/// Returns \c true if this properties bag is not equal to \a rhs.
		bool operator!=(const MosaicProperties& rhs) const {
			return !(*this == rhs);
		}

	private:
		std::array<MosaicProperty, Num_Mosaic_Properties> m_properties;
	};

	/// Extracts all properties from \a header and \a pProperties.
	MosaicProperties ExtractAllProperties(const MosaicPropertiesHeader& header, const MosaicProperty* pProperties);
}}
