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
#include "catapult/utils/WrappedWithOwnerDecorator.h"
#include "catapult/exceptions.h"
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	/// Wraps a strand and automatically augments handlers to extend the lifetime of an owning object.
	template<typename TOwner>
	class StrandOwnerLifetimeExtender {
	private:
#ifdef ENABLE_CATAPULT_DIAGNOSTICS
		class outside_strand_error : public catapult_runtime_error {
		public:
			using catapult_runtime_error::catapult_runtime_error;
		};

		template<typename THandler>
		class StrandCheckHandler {
		public:
			StrandCheckHandler(const boost::asio::io_context::strand& strand, THandler handler)
					: m_pStrand(&strand)
					, m_handler(std::move(handler))
			{}

		public:
			template<typename... TArgs>
			auto operator()(TArgs&& ...args) const {
				if (!m_pStrand->running_in_this_thread())
					CATAPULT_THROW_AND_LOG_0(outside_strand_error, "wrapped function executing outside of strand");

				return m_handler(std::forward<TArgs>(args)...);
			}

		protected:
			const boost::asio::io_context::strand* m_pStrand;
			THandler m_handler;
		};

		template<typename THandler>
		static auto MakeStrandCheckHandler(const boost::asio::io_context::strand& strand, THandler handler) {
			return StrandCheckHandler<THandler>(strand, std::move(handler));
		}
#else
		template<typename THandler>
		static auto MakeStrandCheckHandler(const boost::asio::io_context::strand&, THandler handler) {
			return std::move(handler);
		}
#endif

	public:
		/// Creates a strand owner lifetime extender around \a strand.
		explicit StrandOwnerLifetimeExtender(boost::asio::io_context::strand& strand) : m_strand(strand)
		{}

	public:
		/// Wraps \a handler and attaches \a pOwner to it.
		template<typename THandler>
		auto wrap(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// 1. check in-strand execution (in diagnostic mode)
			auto checkedHandler = MakeStrandCheckHandler<THandler>(m_strand, std::move(handler));

			// 2. extend owner lifetime
			utils::WrappedWithOwnerDecorator<decltype(checkedHandler)> wrappedWithOwnerHandler(pOwner, std::move(checkedHandler));

			// 3. specify strand as executor for asio
			return boost::asio::bind_executor(m_strand, std::move(wrappedWithOwnerHandler));
		}

	public:
		/// Attaches \a pOwner to \a handler and dispatches it to the strand.
		template<typename THandler>
		void dispatch(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// ensure all handlers extend the lifetime of pOwner and dispatch to a strand
			boost::asio::dispatch(m_strand, [pOwner, handler]() {
				handler(pOwner);
			});
		}

		/// Attaches \a pOwner to \a handler and posts it to the strand.
		template<typename THandler>
		void post(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// ensure all handlers extend the lifetime of pOwner and post to a strand
			boost::asio::post(m_strand, [pOwner, handler]() {
				handler(pOwner);
			});
		}

	private:
		boost::asio::io_context::strand& m_strand;
	};
}}
