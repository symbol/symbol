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
#include "catapult/utils/ArraySet.h"
#include "catapult/plugins.h"

namespace catapult { namespace state {

	using SortedAddressSet = std::set<Address>;

	/// Mixin for storing information about cosignatories of an account.
	class MultisigCosignatoriesMixin {
	public:
		/// Creates multisig cosignatories mixin.
		MultisigCosignatoriesMixin()
				: m_minApproval(0)
				, m_minRemoval(0)
		{}

	public:
		/// Gets the (const) cosignatory addresses.
		const SortedAddressSet& cosignatoryAddresses() const {
			return m_cosignatoryAddresses;
		}

		/// Gets the cosignatory addresses.
		SortedAddressSet& cosignatoryAddresses() {
			return m_cosignatoryAddresses;
		}

		/// Returns \c true if \a address is a cosignatory.
		bool hasCosignatory(const Address& address) const {
			return m_cosignatoryAddresses.end() != m_cosignatoryAddresses.find(address);
		}

		/// Gets the number of cosignatories required when approving (any) transaction.
		uint32_t minApproval() const {
			return m_minApproval;
		}

		/// Sets the number of cosignatories required (\a minApproval) when approving (any) transaction.
		void setMinApproval(uint32_t minApproval) {
			m_minApproval = minApproval;
		}

		/// Gets the number of cosignatories required when removing an account.
		uint32_t minRemoval() const {
			return m_minRemoval;
		}

		/// Sets the number of cosignatories required (\a minRemoval) when removing an account.
		void setMinRemoval(uint32_t minRemoval) {
			m_minRemoval = minRemoval;
		}

	private:
		SortedAddressSet m_cosignatoryAddresses;
		uint32_t m_minApproval;
		uint32_t m_minRemoval;
	};

	/// Mixin for storing information about accounts that current account can cosign.
	class MultisigCosignatoryOfMixin {
	public:
		/// Gets the (const) multisig addresses.
		const SortedAddressSet& multisigAddresses() const {
			return m_multisigAddresses;
		}

		/// Gets the multisig addresses.
		SortedAddressSet& multisigAddresses() {
			return m_multisigAddresses;
		}

	private:
		SortedAddressSet m_multisigAddresses;
	};

	/// Multisig entry.
	class PLUGIN_API_DEPENDENCY MultisigEntry : public MultisigCosignatoriesMixin, public MultisigCosignatoryOfMixin {
	public:
		/// Creates a multisig entry around \a address.
		explicit MultisigEntry(const Address& address) : m_address(address)
		{}

	public:
		/// Gets the account address.
		const Address& address() const {
			return m_address;
		}

	private:
		Address m_address;
	};
}}
