#pragma once
#include "NamespaceConstants.h"
#include "NamespaceTypes.h"
#include "catapult/model/EntityType.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a register namespace transaction body.
	template<typename THeader>
	struct RegisterNamespaceTransactionBody : public THeader {
	private:
		using TransactionType = RegisterNamespaceTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 2;

		/// Transaction type.
		static constexpr EntityType Entity_Type = EntityType::Register_Namespace;

	public:
		/// Creates a register namespace transaction body.
		/// \note Explicit default constructor is needed because body contains a union.
		RegisterNamespaceTransactionBody() {}

	public:
		/// The type of namespace being registered.
		model::NamespaceType NamespaceType;

		union {
			/// The id of a parent namespace.
			/// \note This field is only valid when NamespaceType is Child.
			NamespaceId ParentId;

			/// The amount of blocks for which a namespace should be valid.
			/// \note This field is only valid when NamespaceType is Root.
			ArtifactDuration Duration;
		};

		/// The id of the namespace.
		/// \note This must match the generated id.
		catapult::NamespaceId NamespaceId;

		/// The size of a namespace name.
		uint8_t NamespaceNameSize;

		// followed by namespace name

	private:
		template<typename T>
		static auto NamePtrT(T& transaction) {
			return transaction.NamespaceNameSize ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Returns a const pointer to the namespace part name.
		const uint8_t* NamePtr() const {
			return NamePtrT(*this);
		}

		/// Returns a pointer to the namespace part name.
		uint8_t* NamePtr() {
			return NamePtrT(*this);
		}

	public:
		/// Returns \c true if this transaction registers a root namespace.
		bool IsRootRegistration() const {
			return NamespaceType::Root == NamespaceType;
		}

		/// Returns \c true if this transaction registers a child namespace.
		bool IsChildRegistration() const {
			return NamespaceType::Child == NamespaceType;
		}

	public:
		/// Calculates the real size of register namespace \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.NamespaceNameSize;
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(RegisterNamespace)

#pragma pack(pop)
}}
