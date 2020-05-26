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
#include "src/state/AccountRestriction.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	// region restriction traits

	struct BaseAccountAddressRestrictionTraits {
		using UnresolvedValueType = UnresolvedAddress;
		using ValueType = Address;

		static constexpr auto Restriction_Flags = model::AccountRestrictionFlags::Address;
		static constexpr auto Restriction_Value_Size = Address::Size;

		static auto RandomUnresolvedValue() {
			return test::GenerateRandomUnresolvedAddress();
		}

		static auto RandomValue() {
			return test::GenerateRandomByteArray<Address>();
		}

		static UnresolvedValueType Unresolve(const ValueType& value) {
			return test::UnresolveXor(value);
		}

		static auto FromBuffer(const RawBuffer& buffer) {
			UnresolvedValueType address;
			std::memcpy(address.data(), buffer.pData, buffer.Size);
			return address;
		}
	};

	struct BaseAccountMosaicRestrictionTraits {
		using UnresolvedValueType = UnresolvedMosaicId;
		using ValueType = MosaicId;

		static constexpr auto Restriction_Flags = model::AccountRestrictionFlags::MosaicId;
		static constexpr auto Restriction_Value_Size = sizeof(ValueType);

		static auto RandomUnresolvedValue() {
			return test::GenerateRandomValue<UnresolvedValueType>();
		}

		static auto RandomValue() {
			return test::GenerateRandomValue<ValueType>();
		}

		static UnresolvedValueType Unresolve(const ValueType& value) {
			return test::UnresolveXor(value);
		}

		static auto FromBuffer(const RawBuffer& buffer) {
			return reinterpret_cast<const UnresolvedValueType&>(*buffer.pData);
		}
	};

	struct BaseAccountOperationRestrictionTraits {
		using UnresolvedValueType = model::EntityType;
		using ValueType = model::EntityType;

		static constexpr auto Restriction_Flags = model::AccountRestrictionFlags::TransactionType
				| model::AccountRestrictionFlags::Outgoing;
		static constexpr auto Restriction_Value_Size = sizeof(ValueType);

		static auto RandomUnresolvedValue() {
			return static_cast<UnresolvedValueType>(test::RandomByte());
		}

		static auto RandomValue() {
			return static_cast<ValueType>(test::RandomByte());
		}

		static UnresolvedValueType Unresolve(const ValueType& value) {
			return value;
		}

		static auto FromBuffer(const RawBuffer& buffer) {
			return reinterpret_cast<const UnresolvedValueType&>(*buffer.pData);
		}
	};

	// endregion

	// region allow/block traits

	/// Traits for operation type 'Allow'.
	struct AllowTraits {
		/// Given \a restrictionFlags gets the restriction flags including the operation type.
		static model::AccountRestrictionFlags CompleteAccountRestrictionFlags(model::AccountRestrictionFlags restrictionFlags) {
			return restrictionFlags;
		}

		/// Given \a restrictionFlags gets the restriction flags including the opposite operation type.
		static model::AccountRestrictionFlags OppositeCompleteAccountRestrictionFlags(model::AccountRestrictionFlags restrictionFlags) {
			return restrictionFlags | model::AccountRestrictionFlags::Block;
		}

		/// Adds \a value to \a restriction for operation type 'Allow'.
		static void Add(state::AccountRestriction& restriction, const state::AccountRestriction::RawValue& value) {
			restriction.allow({ model::AccountRestrictionModificationAction::Add, value });
		}
	};

	/// Traits for operation type 'Block'.
	struct BlockTraits {
		/// Given \a restrictionFlags gets the restriction flags including the operation type.
		static model::AccountRestrictionFlags CompleteAccountRestrictionFlags(model::AccountRestrictionFlags restrictionFlags) {
			return restrictionFlags | model::AccountRestrictionFlags::Block;
		}

		/// Given \a restrictionFlags gets the restriction flags including the opposite operation type.
		static model::AccountRestrictionFlags OppositeCompleteAccountRestrictionFlags(model::AccountRestrictionFlags restrictionFlags) {
			return restrictionFlags;
		}

		/// Adds \a value to \a restriction for operation type 'Block'.
		static void Add(state::AccountRestriction& restriction, const state::AccountRestriction::RawValue& value) {
			restriction.block({ model::AccountRestrictionModificationAction::Add, value });
		}
	};

	// endregion

	// region CreateNotification

	/// Creates an account restriction value notification around \a address, \a restrictionValue and \a action.
	template<typename TRestrictionValueTraits, typename TOperationTraits = AllowTraits>
	auto CreateAccountRestrictionValueNotification(
			const Address& address,
			const typename TRestrictionValueTraits::UnresolvedValueType& restrictionValue,
			model::AccountRestrictionModificationAction action) {
		return typename TRestrictionValueTraits::NotificationType(
				address,
				TOperationTraits::CompleteAccountRestrictionFlags(TRestrictionValueTraits::Restriction_Flags),
				restrictionValue,
				action);
	}

	/// Creates an account restrictions notification around \a address, \a restrictionAdditions and \a restrictionDeletions.
	template<typename TRestrictionValueTraits, typename TValueType, typename TOperationTraits = AllowTraits>
	auto CreateAccountRestrictionsNotification(
			const Address& address,
			const std::vector<TValueType>& restrictionAdditions,
			const std::vector<TValueType>& restrictionDeletions) {
		return typename TRestrictionValueTraits::NotificationType(
				address,
				TOperationTraits::CompleteAccountRestrictionFlags(TRestrictionValueTraits::Restriction_Flags),
				utils::checked_cast<size_t, uint8_t>(restrictionAdditions.size()),
				restrictionAdditions.data(),
				utils::checked_cast<size_t, uint8_t>(restrictionDeletions.size()),
				restrictionDeletions.data());
	}

	// endregion
}}
