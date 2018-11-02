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

#include "AccountStateSerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace state {

	// region AccountStateNonHistoricalSerializer

	void AccountStateNonHistoricalSerializer::Save(const AccountState& accountState, io::OutputStream& output) {
		// write identifying information
		io::Write(output, accountState.Address);
		io::Write(output, accountState.AddressHeight);
		io::Write(output, accountState.PublicKey);
		io::Write(output, accountState.PublicKeyHeight);

		// write link information
		io::Write8(output, utils::to_underlying_type(accountState.AccountType));
		io::Write(output, accountState.LinkedAccountKey);

		// write last importance
		io::Write(output, accountState.ImportanceInfo.current());
		io::Write(output, accountState.ImportanceInfo.height());

		// write mosaics
		io::Write16(output, static_cast<uint16_t>(accountState.Balances.size()));
		for (const auto& pair : accountState.Balances) {
			io::Write(output, pair.first);
			io::Write(output, pair.second);
		}
	}

	namespace {
		AccountImportance::ImportanceSnapshot ReadImportanceSnapshot(io::InputStream& input) {
			AccountImportance::ImportanceSnapshot snapshot;
			snapshot.Importance = io::Read<Importance>(input);
			snapshot.Height = io::Read<model::ImportanceHeight>(input);
			return snapshot;
		}

		void SetImportance(AccountState& accountState, const AccountImportance::ImportanceSnapshot& snapshot) {
			if (model::ImportanceHeight() != snapshot.Height)
				accountState.ImportanceInfo.set(snapshot.Importance, snapshot.Height);
		}

		AccountState LoadAccountStateWithoutHistory(io::InputStream& input, AccountImportance::ImportanceSnapshot& lastSnapshot) {
			// read identifying information
			Address address;
			io::Read(input, address);
			auto addressHeight = io::Read<Height>(input);

			auto accountState = AccountState(address, addressHeight);

			io::Read(input, accountState.PublicKey);
			accountState.PublicKeyHeight = io::Read<Height>(input);

			// read link information
			accountState.AccountType = static_cast<state::AccountType>(io::Read8(input));
			io::Read(input, accountState.LinkedAccountKey);

			// read last importance
			lastSnapshot = ReadImportanceSnapshot(input);

			// read mosaics
			auto numMosaics = io::Read16(input);
			for (auto i = 0u; i < numMosaics; ++i) {
				auto mosaicId = io::Read<MosaicId>(input);
				auto amount = io::Read<Amount>(input);
				accountState.Balances.credit(mosaicId, amount);
			}

			return accountState;
		}
	}

	AccountState AccountStateNonHistoricalSerializer::Load(io::InputStream& input) {
		AccountImportance::ImportanceSnapshot lastSnapshot;
		auto accountState = LoadAccountStateWithoutHistory(input, lastSnapshot);

		SetImportance(accountState, lastSnapshot);
		return accountState;
	}

	// endregion

	// region AccountStateSerializer

	namespace {
		void FillImportanceSnapshots(const AccountImportance& accountImportance, AccountImportance::ImportanceSnapshot* pSnapshot) {
			for (const auto& snapshot : accountImportance)
				*pSnapshot++ = snapshot;
		}
	}

	void AccountStateSerializer::Save(const AccountState& accountState, io::OutputStream& output) {
		// write non-historical information
		AccountStateNonHistoricalSerializer::Save(accountState, output);

		// write historical importances (reverse order)
		AccountImportance::ImportanceSnapshot snapshots[Importance_History_Size];
		FillImportanceSnapshots(accountState.ImportanceInfo, snapshots);

		for (auto i = Importance_History_Size; i > 1; --i) {
			const auto& snapshot = snapshots[i - 1];
			io::Write(output, snapshot.Importance);
			io::Write(output, snapshot.Height);
		}
	}

	AccountState AccountStateSerializer::Load(io::InputStream& input) {
		// read non-historical information
		AccountImportance::ImportanceSnapshot lastSnapshot;
		auto accountState = LoadAccountStateWithoutHistory(input, lastSnapshot);

		// read historical importances and set all importances
		for (auto i = 0u; i < Importance_History_Size - 1; ++i)
			SetImportance(accountState, ReadImportanceSnapshot(input));

		SetImportance(accountState, lastSnapshot);
		return accountState;
	}

	// endregion
}}
