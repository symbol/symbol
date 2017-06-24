#pragma once
#include "ImportanceHeight.h"
#include "Mosaic.h"
#include "catapult/constants.h"
#include "catapult/types.h"
#include <cstring>
#include <memory>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an account info.
	struct AccountInfo {
		/// Size of the account info.
		uint32_t Size;

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
		model::ImportanceHeight ImportanceHeights[Importance_History_Size];

		/// The number of mosaics the account owns.
		uint16_t NumMosaics;

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

	private:
		static const uint8_t* ToBytePointer(const AccountInfo& accountInfo) {
			return reinterpret_cast<const uint8_t*>(&accountInfo);
		}

		static uint8_t* ToBytePointer(AccountInfo& accountInfo) {
			return reinterpret_cast<uint8_t*>(&accountInfo);
		}

		template<typename T>
		static auto PayloadStart(T& accountInfo) {
			return accountInfo.Size != CalculateRealSize(accountInfo) ? nullptr : ToBytePointer(accountInfo) + sizeof(T);
		}

		static const Mosaic* ToMosaicPointer(const uint8_t* pData) {
			return reinterpret_cast<const Mosaic*>(pData);
		}

		static Mosaic* ToMosaicPointer(uint8_t* pData) {
			return reinterpret_cast<Mosaic*>(pData);
		}

	public:
		/// Returns a const pointer to the first mosaic contained in this account info.
		const Mosaic* MosaicsPtr() const {
			return NumMosaics ? ToMosaicPointer(PayloadStart(*this)) : nullptr;
		}

		/// Returns a pointer to the first mosaic contained in this account info.
		Mosaic* MosaicsPtr() {
			return NumMosaics ? ToMosaicPointer(PayloadStart(*this)) : nullptr;
		}

	private:
		static constexpr uint64_t CalculateRealSize(const AccountInfo& accountInfo) noexcept {
			return sizeof(AccountInfo) + accountInfo.NumMosaics * sizeof(Mosaic);
		}

		friend constexpr uint64_t CalculateRealSize(const AccountInfo& accountInfo) noexcept;
	};

#pragma pack(pop)

	/// Maximum size of AccountInfo containing maximum allowed number of mosaics.
	constexpr auto AccountInfo_Max_Size =
			sizeof(model::AccountInfo) + sizeof(model::Mosaic) * ((1 << (8 * sizeof(model::AccountInfo().NumMosaics))) - 1);

	/// Calculates the real size of \a accountInfo.
	constexpr uint64_t CalculateRealSize(const AccountInfo& accountInfo) noexcept {
		return AccountInfo::CalculateRealSize(accountInfo);
	}

	/// Checks the real size of \a accountInfo against its reported size and returns \c true if the sizes match.
	constexpr bool IsSizeValid(const AccountInfo& accountInfo) noexcept {
		return CalculateRealSize(accountInfo) == accountInfo.Size;
	}
}}
