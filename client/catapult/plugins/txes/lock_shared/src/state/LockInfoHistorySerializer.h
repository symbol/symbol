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
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	/// Policy for saving and loading lock info data.
	template<typename TLockInfo, typename TLockInfoSerializer>
	struct LockInfoSerializer {
	public:
		/// Saves \a lockInfo to \a output.
		static void Save(const TLockInfo& lockInfo, io::OutputStream& output) {
			output.write(lockInfo.OwnerAddress);
			io::Write(output, lockInfo.MosaicId);
			io::Write(output, lockInfo.Amount);
			io::Write(output, lockInfo.EndHeight);
			io::Write8(output, utils::to_underlying_type(lockInfo.Status));
			TLockInfoSerializer::Save(lockInfo, output);
		}

		/// Loads a single value from \a input.
		static TLockInfo Load(io::InputStream& input) {
			TLockInfo lockInfo;
			input.read(lockInfo.OwnerAddress);
			io::Read(input, lockInfo.MosaicId);
			io::Read(input, lockInfo.Amount);
			io::Read(input, lockInfo.EndHeight);
			lockInfo.Status = static_cast<LockStatus>(io::Read8(input));
			TLockInfoSerializer::Load(input, lockInfo);
			return lockInfo;
		}
	};

	/// Policy for saving and loading lock info history data without historical information.
	template<typename TLockInfoHistory, typename TLockInfoSerializer>
	struct LockInfoHistoryNonHistoricalSerializer {
	public:
		/// Saves \a history to \a output.
		static void Save(const TLockInfoHistory& history, io::OutputStream& output) {
			if (history.empty())
				CATAPULT_THROW_RUNTIME_ERROR_1("cannot save empty lock info history", history.id());

			TLockInfoSerializer::Save(history.back(), output);
		}

		/// Loads a single value from \a input.
		static TLockInfoHistory Load(io::InputStream& input) {
			auto lockInfo = TLockInfoSerializer::Load(input);
			auto history = TLockInfoHistory(GetLockIdentifier(lockInfo));
			history.push_back(lockInfo);
			return history;
		}
	};

	/// Policy for saving and loading lock info history data.
	template<typename TLockInfoHistory, typename TLockInfoSerializer>
	struct LockInfoHistorySerializer {
	public:
		/// Saves \a history to \a output.
		static void Save(const TLockInfoHistory& history, io::OutputStream& output) {
			io::Write64(output, history.historyDepth());
			output.write(history.id());

			for (const auto& lockInfo : history)
				TLockInfoSerializer::Save(lockInfo, output);
		}

		/// Loads a single value from \a input.
		static TLockInfoHistory Load(io::InputStream& input) {
			auto count = io::Read64(input);

			Hash256 id;
			input.read(id);

			auto history = TLockInfoHistory(id);
			for (auto i = 0u; i < count; ++i)
				history.push_back(TLockInfoSerializer::Load(input));

			return history;
		}
	};
}}

/// Defines lock info history serializers for \a LOCK_INFO with state \a VERSION.
#define DEFINE_LOCK_INFO_HISTORY_SERIALIZERS(LOCK_INFO, VERSION) \
	using LOCK_INFO##Serializer = LockInfoSerializer<LOCK_INFO, LOCK_INFO##ExtendedDataSerializer>; \
	\
	struct LOCK_INFO##HistoryNonHistoricalSerializer \
			: public LockInfoHistoryNonHistoricalSerializer<LOCK_INFO##History, LOCK_INFO##Serializer> { \
		static constexpr uint16_t State_Version = VERSION; \
	}; \
	\
	struct LOCK_INFO##HistorySerializer : public LockInfoHistorySerializer<LOCK_INFO##History, LOCK_INFO##Serializer> { \
		static constexpr uint16_t State_Version = VERSION; \
	};
