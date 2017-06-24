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
		/// The number of active worker threads.
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
	/// optional friendly name used in logging (\a pName).
	std::unique_ptr<IoServiceThreadPool> CreateIoServiceThreadPool(size_t numWorkerThreads, const char* pName = nullptr);
}}
