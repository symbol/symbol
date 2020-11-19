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
#include <memory>
#include <string>

namespace boost { namespace asio { class io_context; } }

namespace catapult { namespace thread {

	/// Represents a thread pool that shares a single io context across multiple threads.
	class IoThreadPool {
	public:
		virtual ~IoThreadPool() = default;

	public:
		/// Gets the number of active worker threads.
		virtual uint32_t numWorkerThreads() const = 0;

		/// Gets the friendly name of this thread pool.
		virtual const std::string& name() const = 0;

		/// Gets the underlying io_context.
		virtual boost::asio::io_context& ioContext() = 0;

	public:
		/// Starts the thread pool.
		/// \note All worker threads will be active when this function returns.
		virtual void start() = 0;

		/// Waits for all thread pool threads to exit.
		virtual void join() = 0;
	};

	/// Creates an io thread pool with the specified number of threads (\a numWorkerThreads).
	/// Optional friendly \a name can be provided to tag logs.
	std::unique_ptr<IoThreadPool> CreateIoThreadPool(size_t numWorkerThreads, const char* name = nullptr);
}}
