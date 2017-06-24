#pragma once
#include "AccountBalances.h"
#include "AccountImportance.h"
#include "catapult/model/AccountInfo.h"
#include "catapult/utils/Casting.h"
#include <memory>

namespace catapult { namespace state {

	/// Account state data.
	struct AccountState {
	public:
		/// Creates an account state from an \a address and a height (\a addressHeight).
		explicit AccountState(const catapult::Address& address, Height addressHeight)
				: Address(address)
				, AddressHeight(addressHeight)
				, PublicKeyHeight(0)
		{}

		/// Creates an account state from \a info.
		explicit AccountState(const model::AccountInfo& info)
				: Address(info.Address)
				, AddressHeight(info.AddressHeight)
				, PublicKey(info.PublicKey)
				, PublicKeyHeight(info.PublicKeyHeight) {
			for (auto i = Importance_History_Size; i > 0; --i) {
				auto importanceHeight = info.ImportanceHeights[i - 1];
				if (model::ImportanceHeight() == importanceHeight)
					continue;

				ImportanceInfo.set(info.Importances[i - 1], importanceHeight);
			}

			auto pMosaic = info.MosaicsPtr();
			for (auto i = 0u; i < info.NumMosaics; ++i, ++pMosaic)
				Balances.credit(pMosaic->MosaicId, pMosaic->Amount);
		}

	public:
		/// Address of an account.
		catapult::Address Address;

		/// Height at which address has been obtained.
		Height AddressHeight;

		/// Public key of an account. Present if PublicKeyHeight > 0.
		Key PublicKey;

		/// Height at which public key has been obtained.
		Height PublicKeyHeight;

		/// Importance information of the account.
		AccountImportance ImportanceInfo;

		/// Balances of an account.
		AccountBalances Balances;

	public:
		/// Creates an account info from this account state.
		std::shared_ptr<model::AccountInfo> toAccountInfo() const {
			auto numMosaics = utils::checked_cast<size_t, uint16_t>(Balances.size());
			uint32_t entitySize = sizeof(model::AccountInfo) + numMosaics * sizeof(model::Mosaic);
			std::shared_ptr<model::AccountInfo> pAccountInfo(reinterpret_cast<model::AccountInfo*>(::operator new(entitySize)));
			pAccountInfo->Size = entitySize;
			pAccountInfo->Address = Address;
			pAccountInfo->AddressHeight = AddressHeight;
			pAccountInfo->PublicKey = PublicKey;
			pAccountInfo->PublicKeyHeight = PublicKeyHeight;

			auto i = 0u;
			for (const auto& pair : ImportanceInfo) {
				pAccountInfo->Importances[i] = pair.Importance;
				pAccountInfo->ImportanceHeights[i] = pair.Height;
				++i;
			}

			pAccountInfo->NumMosaics = numMosaics;
			auto pMosaic = pAccountInfo->MosaicsPtr();
			for (const auto& pair : Balances) {
				pMosaic->MosaicId = pair.first;
				pMosaic->Amount = pair.second;
				++pMosaic;
			}

			return pAccountInfo;
		}
	};
}}
