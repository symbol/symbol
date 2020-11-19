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
#include "NamespaceConstants.h"
#include "NamespaceEntityType.h"
#include "NamespaceTypes.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a namespace registration transaction body.
	template<typename THeader>
	struct NamespaceRegistrationTransactionBody : public THeader {
	private:
		using TransactionType = NamespaceRegistrationTransactionBody<THeader>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Namespace_Registration, 1)

	public:
		/// Creates a namespace registration transaction body.
		/// \note Explicit default constructor is needed because body contains a union.
		NamespaceRegistrationTransactionBody()
		{}

	public:
		union {
			/// Parent namespace identifier.
			/// \note This field is only valid when RegistrationType is Child.
			NamespaceId ParentId;

			/// Number of blocks for which the namespace should be valid.
			/// \note This field is only valid when RegistrationType is Root.
			BlockDuration Duration;
		};

		/// Namespace identifier.
		/// \note This must match the generated id.
		NamespaceId Id;

		/// Namespace registration type.
		NamespaceRegistrationType RegistrationType;

		/// Namespace name size.
		uint8_t NameSize;

		// followed by namespace name
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Name, uint8_t)

	private:
		template<typename T>
		static auto* NamePtrT(T& transaction) {
			return transaction.NameSize ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Returns \c true if this transaction registers a root namespace.
		bool IsRootRegistration() const {
			return NamespaceRegistrationType::Root == RegistrationType;
		}

		/// Returns \c true if this transaction registers a child namespace.
		bool IsChildRegistration() const {
			return NamespaceRegistrationType::Child == RegistrationType;
		}

	public:
		/// Calculates the real size of namespace registration \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.NameSize;
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(NamespaceRegistration)

#pragma pack(pop)
}}
