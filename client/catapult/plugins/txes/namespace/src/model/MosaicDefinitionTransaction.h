#pragma once
#include "MosaicProperties.h"
#include "NamespaceConstants.h"
#include "catapult/model/EntityType.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a mosaic definition transaction body.
	template<typename THeader>
	struct MosaicDefinitionTransactionBody : public THeader {
	private:
		using TransactionType = MosaicDefinitionTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 2;

		/// Transaction type.
		static constexpr EntityType Entity_Type = EntityType::Mosaic_Definition;

	public:
		/// The id of a parent namespace.
		NamespaceId ParentId;

		/// The id of the mosaic.
		/// \note This must match the generated id.
		catapult::MosaicId MosaicId;

		/// The size of a mosaic name.
		uint8_t MosaicNameSize;

		/// Properties header.
		MosaicPropertiesHeader PropertiesHeader;

		// followed by mosaic name

		// followed by optional properties

	private:
		template<typename T>
		static auto NamePtrT(T& transaction) {
			return transaction.MosaicNameSize ? THeader::PayloadStart(transaction) : nullptr;
		}

		template<typename T>
		static auto PropertiesPtrT(T& transaction) {
			auto* pPayloadStart = THeader::PayloadStart(transaction);
			return transaction.PropertiesHeader.Count && pPayloadStart
					? pPayloadStart + transaction.MosaicNameSize
					: nullptr;
		}

	public:
		/// Returns a const pointer to the mosaic name.
		const uint8_t* NamePtr() const {
			return NamePtrT(*this);
		}

		/// Returns a pointer to the mosaic name.
		uint8_t* NamePtr() {
			return NamePtrT(*this);
		}

		/// Returns a const pointer to optional properties.
		const MosaicProperty* PropertiesPtr() const {
			return reinterpret_cast<const MosaicProperty*>(PropertiesPtrT(*this));
		}

		/// Returns a pointer to optional properties.
		MosaicProperty* PropertiesPtr() {
			return reinterpret_cast<MosaicProperty*>(PropertiesPtrT(*this));
		}

	public:
		/// Calculates the real size of mosaic definition \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType)
					+ transaction.PropertiesHeader.Count * sizeof(model::MosaicProperty)
					+ transaction.MosaicNameSize;
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(MosaicDefinition)

#pragma pack(pop)
}}
