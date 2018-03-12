#pragma once
#include "ImportanceHeight.h"
#include "Mosaic.h"
#include "TrailingVariableDataLayout.h"
#include "catapult/constants.h"
#include "catapult/types.h"
#include <cstring>
#include <memory>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an account info.
	struct AccountInfo : public TrailingVariableDataLayout<AccountInfo, Mosaic> {
	public:
		/// Address of the account.
		catapult::Address Address;

		/// Height at which address has been obtained.
		Height AddressHeight;

		/// Public key of the account. Valid if PublicKeyHeight > 0.
		Key PublicKey;

		/// Height at which public key has been obtained.
		Height PublicKeyHeight;

		/// Importances of the account (current followed by historical).
		catapult::Importance Importances[Importance_History_Size];

		/// Importance heights (current followed by historical).
		ImportanceHeight ImportanceHeights[Importance_History_Size];

		/// Number of types of mosaics owned by the account.
		uint16_t MosaicsCount;

		// followed by mosaics data if MosaicsCount != 0

	public:
		/// Returns a const pointer to the first mosaic contained in this account info.
		const Mosaic* MosaicsPtr() const {
			return MosaicsCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

		/// Returns a pointer to the first mosaic contained in this account info.
		Mosaic* MosaicsPtr() {
			return MosaicsCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

	public:
		/// Creates a zero-initialized account info with \a address.
		static std::shared_ptr<const AccountInfo> FromAddress(const catapult::Address& address) {
			uint32_t entitySize = sizeof(AccountInfo);
			auto pAccountInfo = std::make_shared<AccountInfo>();
			std::memset(pAccountInfo.get(), 0, entitySize);
			pAccountInfo->Size = entitySize;
			pAccountInfo->Address = address;
			return pAccountInfo;
		}

	public:
		/// Calculates the real size of \a accountInfo.
		static constexpr uint64_t CalculateRealSize(const AccountInfo& accountInfo) noexcept {
			return sizeof(AccountInfo) + accountInfo.MosaicsCount * sizeof(Mosaic);
		}
	};

#pragma pack(pop)

	/// Maximum size of AccountInfo containing maximum allowed number of mosaics.
	constexpr auto AccountInfo_Max_Size = sizeof(AccountInfo) + sizeof(Mosaic) * ((1 << (8 * sizeof(AccountInfo::MosaicsCount))) - 1);
}}
