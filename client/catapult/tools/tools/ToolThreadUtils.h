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
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace thread { class IoThreadPool; } }

namespace catapult { namespace tools {

	/// Gets the value associated with \a future and forwards it to \a action on success.
	/// On failure, \c false is returned and \a description is included in a warning log.
	template<typename TAction, typename TFuture>
	bool UnwrapFutureAndSuppressErrors(const char* description, TFuture&& future, TAction action) {
		try {
			action(future.get());
			return true;
		} catch (...) {
			// suppress
			CATAPULT_LOG(warning) << UNHANDLED_EXCEPTION_MESSAGE(description);
			return false;
		}
	}

	/// Creates a started thread pool with \a numThreads threads.
	std::unique_ptr<thread::IoThreadPool> CreateStartedThreadPool(uint32_t numThreads = 1);
}}
