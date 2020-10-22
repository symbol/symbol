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

#include "AccountStateSerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace state {

	namespace {
		// region AccountStateFormat

		enum class AccountStateFormat : uint8_t { Regular = 0, High_Value = 1 };

		AccountStateFormat GetFormat(const AccountState& accountState) {
			return Importance() == accountState.ImportanceSnapshots.current()
					? AccountStateFormat::Regular
					: AccountStateFormat::High_Value;
		}

		// endregion

		// region ImportanceReader

		class ImportanceReader {
		public:
			ImportanceReader()
					: m_snapshotsIndex(0)
					, m_bucketsIndex(0)
			{}

		public:
			bool hasSnapshots() const {
				return 0 != m_snapshotsIndex;
			}

		public:
			void readSnapshots(io::InputStream& input, size_t count) {
				for (auto i = 0u ; i < count; ++i) {
					auto& snapshot = m_snapshots[Importance_History_Size - 1 - m_snapshotsIndex];
					snapshot.Importance = io::Read<Importance>(input);
					snapshot.Height = io::Read<model::ImportanceHeight>(input);
					++m_snapshotsIndex;
				}
			}

			void readBuckets(io::InputStream& input, size_t count) {
				for (auto i = 0u ; i < count; ++i) {
					auto& bucket = m_buckets[Activity_Bucket_History_Size - 1 - m_bucketsIndex];
					bucket.StartHeight = io::Read<model::ImportanceHeight>(input);
					bucket.TotalFeesPaid = io::Read<Amount>(input);
					bucket.BeneficiaryCount = io::Read32(input);
					bucket.RawScore = io::Read64(input);
					++m_bucketsIndex;
				}
			}

		public:
			void apply(AccountState& accountState) {
				for (const auto& snapshot : m_snapshots) {
					if (model::ImportanceHeight() == snapshot.Height)
						continue;

					accountState.ImportanceSnapshots.set(snapshot.Importance, snapshot.Height);
				}

				for (const auto& bucket : m_buckets) {
					if (model::ImportanceHeight() == bucket.StartHeight)
						continue;

					accountState.ActivityBuckets.update(bucket.StartHeight, [&bucket](auto& accountStateBucket) {
						accountStateBucket = bucket;
					});
				}
			}

		private:
			size_t m_snapshotsIndex;
			std::array<AccountImportanceSnapshots::ImportanceSnapshot, Importance_History_Size> m_snapshots;
			size_t m_bucketsIndex;
			std::array<AccountActivityBuckets::ActivityBucket, Activity_Bucket_History_Size> m_buckets;
		};

		// endregion

		// region WriteSnapshots / WriteBuckets

		template<typename TContainer, typename TAction>
		void ProcessRange(const TContainer& container, size_t start, size_t count, TAction action) {
			auto i = 0u;
			for (const auto& value : container) {
				if (i < start) {
					++i;
					continue;
				}

				if (i == start + count)
					break;

				action(value);
				++i;
			}
		}

		void WritePinnedVotingKey(io::OutputStream& output, const model::PinnedVotingKey& pinnedVotingKey) {
			std::array<uint8_t, 16> padding{};
			output.write(pinnedVotingKey.VotingKey);
			output.write(padding);
			io::Write(output, pinnedVotingKey.StartEpoch);
			io::Write(output, pinnedVotingKey.EndEpoch);
		}

		void WriteSupplementalPublicKeys(io::OutputStream& output, const AccountPublicKeys& accountPublicKeys) {
			io::Write8(output, utils::to_underlying_type(accountPublicKeys.mask()));
			io::Write8(output, static_cast<uint8_t>(accountPublicKeys.voting().size()));

			if (HasFlag(AccountPublicKeys::KeyType::Linked, accountPublicKeys.mask()))
				output.write(accountPublicKeys.linked().get());

			if (HasFlag(AccountPublicKeys::KeyType::Node, accountPublicKeys.mask()))
				output.write(accountPublicKeys.node().get());

			if (HasFlag(AccountPublicKeys::KeyType::VRF, accountPublicKeys.mask()))
				output.write(accountPublicKeys.vrf().get());

			for (auto i = 0u; i < accountPublicKeys.voting().size(); ++i) {
				const auto& pinnedVotingKey = accountPublicKeys.voting().get(i);
				WritePinnedVotingKey(output, pinnedVotingKey);
			}
		}

		void WriteSnapshots(io::OutputStream& output, const AccountImportanceSnapshots& snapshots, size_t start, size_t count) {
			ProcessRange(snapshots, start, count, [&output](const auto& snapshot) {
				io::Write(output, snapshot.Importance);
				io::Write(output, snapshot.Height);
			});
		}

		void WriteBuckets(io::OutputStream& output, const AccountActivityBuckets& buckets, size_t start, size_t count) {
			ProcessRange(buckets, start, count, [&output](const auto& bucket) {
				io::Write(output, bucket.StartHeight);
				io::Write(output, bucket.TotalFeesPaid);
				io::Write32(output, bucket.BeneficiaryCount);
				io::Write64(output, bucket.RawScore);
			});
		}

		// endregion
	}

	// region AccountStateNonHistoricalSerializer

	void AccountStateNonHistoricalSerializer::Save(const AccountState& accountState, io::OutputStream& output) {
		// write identifying information
		output.write(accountState.Address);
		io::Write(output, accountState.AddressHeight);
		output.write(accountState.PublicKey);
		io::Write(output, accountState.PublicKeyHeight);

		// write account attributes
		io::Write8(output, utils::to_underlying_type(accountState.AccountType));

		auto format = GetFormat(accountState);
		io::Write8(output, utils::to_underlying_type(format));

		// write supplemental public keys
		WriteSupplementalPublicKeys(output, accountState.SupplementalPublicKeys);

		// write importance information for high value accounts
		if (AccountStateFormat::High_Value == format) {
			WriteSnapshots(output, accountState.ImportanceSnapshots, 0, Importance_History_Size - Rollback_Buffer_Size);
			WriteBuckets(output, accountState.ActivityBuckets, 0, Activity_Bucket_History_Size - Rollback_Buffer_Size);
		}

		// write mosaics
		io::Write16(output, static_cast<uint16_t>(accountState.Balances.size()));
		for (const auto& pair : accountState.Balances) {
			io::Write(output, pair.first);
			io::Write(output, pair.second);
		}
	}

	namespace {
		void ReadSupplementalPublicKey(io::InputStream& input, AccountPublicKeys::PublicKeyAccessor<Key>& publicKeyAccessor) {
			Key key;
			input.read(key);
			publicKeyAccessor.set(key);
		}

		void ReadSupplementalPublicKey(
				io::InputStream& input,
				AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>& publicKeysAccessor) {
			model::PinnedVotingKey key;
			std::array<uint8_t, 16> padding;
			input.read(key.VotingKey);
			input.read(padding);
			io::Read(input, key.StartEpoch);
			io::Read(input, key.EndEpoch);
			publicKeysAccessor.add(key);
		}

		void ReadSupplementalPublicKeys(io::InputStream& input, AccountPublicKeys& accountPublicKeys) {
			auto accountPublicKeysMask = static_cast<AccountPublicKeys::KeyType>(io::Read8(input));
			auto numVotingKeys = io::Read8(input);

			if (HasFlag(AccountPublicKeys::KeyType::Linked, accountPublicKeysMask))
				ReadSupplementalPublicKey(input, accountPublicKeys.linked());

			if (HasFlag(AccountPublicKeys::KeyType::Node, accountPublicKeysMask))
				ReadSupplementalPublicKey(input, accountPublicKeys.node());

			if (HasFlag(AccountPublicKeys::KeyType::VRF, accountPublicKeysMask))
				ReadSupplementalPublicKey(input, accountPublicKeys.vrf());

			for (auto i = 0u; i < numVotingKeys; ++i)
				ReadSupplementalPublicKey(input, accountPublicKeys.voting());
		}

		AccountState LoadAccountStateWithoutHistory(io::InputStream& input, ImportanceReader& importanceReader) {
			// read identifying information
			Address address;
			input.read(address);
			auto addressHeight = io::Read<Height>(input);

			auto accountState = AccountState(address, addressHeight);

			input.read(accountState.PublicKey);
			accountState.PublicKeyHeight = io::Read<Height>(input);

			// read account attributes
			accountState.AccountType = static_cast<AccountType>(io::Read8(input));
			auto format = static_cast<AccountStateFormat>(io::Read8(input));

			// read supplemental public keys
			ReadSupplementalPublicKeys(input, accountState.SupplementalPublicKeys);

			// read importance information for high value accounts
			if (AccountStateFormat::High_Value == format) {
				importanceReader.readSnapshots(input, Importance_History_Size - Rollback_Buffer_Size);
				importanceReader.readBuckets(input, Activity_Bucket_History_Size - Rollback_Buffer_Size);
			} else if (AccountStateFormat::Regular != format) {
				CATAPULT_THROW_INVALID_ARGUMENT_1("cannot load account state with unsupported format", static_cast<uint16_t>(format));
			}

			// read mosaics
			auto numMosaics = io::Read16(input);
			for (auto i = 0u; i < numMosaics; ++i) {
				auto mosaicId = io::Read<MosaicId>(input);
				auto amount = io::Read<Amount>(input);
				accountState.Balances.credit(mosaicId, amount);

				if (0 == i)
					accountState.Balances.optimize(mosaicId);
			}

			return accountState;
		}
	}

	AccountState AccountStateNonHistoricalSerializer::Load(io::InputStream& input) {
		ImportanceReader importanceReader;
		auto accountState = LoadAccountStateWithoutHistory(input, importanceReader);

		importanceReader.apply(accountState);
		return accountState;
	}

	// endregion

	// region AccountStateSerializer

	void AccountStateSerializer::Save(const AccountState& accountState, io::OutputStream& output) {
		// write non-historical information
		AccountStateNonHistoricalSerializer::Save(accountState, output);

		// write historical importance information
		if (AccountStateFormat::High_Value == GetFormat(accountState)) {
			WriteSnapshots(output, accountState.ImportanceSnapshots, Importance_History_Size - Rollback_Buffer_Size, Rollback_Buffer_Size);
			WriteBuckets(output, accountState.ActivityBuckets, Activity_Bucket_History_Size - Rollback_Buffer_Size, Rollback_Buffer_Size);
		} else {
			WriteSnapshots(output, accountState.ImportanceSnapshots, 0, Importance_History_Size);
			WriteBuckets(output, accountState.ActivityBuckets, 0, Activity_Bucket_History_Size);
		}
	}

	AccountState AccountStateSerializer::Load(io::InputStream& input) {
		// read non-historical information
		ImportanceReader importanceReader;
		auto accountState = LoadAccountStateWithoutHistory(input, importanceReader);

		// read historical importance information
		auto isHighValue = importanceReader.hasSnapshots();
		if (isHighValue) {
			importanceReader.readSnapshots(input, Rollback_Buffer_Size);
			importanceReader.readBuckets(input, Rollback_Buffer_Size);
		} else {
			importanceReader.readSnapshots(input, Importance_History_Size);
			importanceReader.readBuckets(input, Activity_Bucket_History_Size);
		}

		importanceReader.apply(accountState);
		return accountState;
	}

	// endregion
}}
