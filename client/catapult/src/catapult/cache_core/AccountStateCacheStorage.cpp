#include "AccountStateCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/state/AccountStateAdapter.h"
#include <vector>

namespace catapult { namespace cache {

	void AccountStateCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		const auto& pAccountState = value.second;
		auto pAccountInfo = state::ToAccountInfo(*pAccountState);
		output.write({ reinterpret_cast<const uint8_t*>(pAccountInfo.get()), pAccountInfo->Size });
	}

	void AccountStateCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta, LoadStateType& state) {
		auto accountInfoSize = io::Read32(input);
		if (accountInfoSize > model::AccountInfo_Max_Size)
			CATAPULT_THROW_RUNTIME_ERROR_1("account in state file has enormous size", accountInfoSize);

		constexpr auto Header_Size = sizeof(accountInfoSize);
		state.resize(accountInfoSize);
		auto pAccountInfo = reinterpret_cast<model::AccountInfo*>(state.data());
		pAccountInfo->Size = accountInfoSize;
		input.read({ state.data() + Header_Size, accountInfoSize - Header_Size });
		cacheDelta.addAccount(*pAccountInfo);
	}
}}
