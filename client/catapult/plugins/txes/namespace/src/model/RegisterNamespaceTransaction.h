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
#include "NamespaceConstants.h"
#include "NamespaceEntityType.h"
#include "NamespaceTypes.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a register namespace transaction body.
	template<typename THeader>
	struct RegisterNamespaceTransactionBody : public THeader {
	private:
		using TransactionType = RegisterNamespaceTransactionBody<THeader>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Register_Namespace, 2)

	public:
		/// Creates a register namespace transaction body.
		/// \note Explicit default constructor is needed because body contains a union.
		RegisterNamespaceTransactionBody()
		{}

	public:
		/// Type of the registered namespace.
		model::NamespaceType NamespaceType;

		union {
			/// Id of the parent namespace.
			/// \note This field is only valid when NamespaceType is Child.
			NamespaceId ParentId;

			/// Number of blocks for which the namespace should be valid.
			/// \note This field is only valid when NamespaceType is Root.
			BlockDuration Duration;
		};

		/// Id of the namespace.
		/// \note This must match the generated id.
		catapult::NamespaceId NamespaceId;

		/// Size of the namespace name.
		uint8_t NamespaceNameSize;

		// followed by namespace name
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Name, uint8_t)

	private:
		template<typename T>
		static auto* NamePtrT(T& transaction) {
			return transaction.NamespaceNameSize ? THeader::PayloadStart(transaction) : nullptr;
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
