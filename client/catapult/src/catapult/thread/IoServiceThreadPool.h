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
#include <memory>
#include <string>

namespace boost { namespace asio { class io_service; } }

namespace catapult { namespace thread {

	/// Represents an io service thread pool that shares a single io service across multiple threads.
	class IoServiceThreadPool {
	public:
		virtual ~IoServiceThreadPool() {}

	public:
		/// Gets the number of active worker threads.
		virtual uint32_t numWorkerThreads() const = 0;

		/// Gets the friendly name of this thread pool.
		virtual const std::string& tag() const = 0;

		/// Gets the underlying io_service.
		virtual boost::asio::io_service& service() = 0;

	public:
		/// Starts the thread pool.
		/// \note All worker threads will be active when this function returns.
		virtual void start() = 0;

		/// Waits for all thread pool threads to exit.
		virtual void join() = 0;
	};

	/// Creates an io service thread pool with the specified number of threads (\a numWorkerThreads) and the
	/// optional friendly \a name used in logging.
	std::unique_ptr<IoServiceThreadPool> CreateIoServiceThreadPool(size_t numWorkerThreads, const char* name = nullptr);
}}
