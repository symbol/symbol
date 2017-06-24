#pragma once
#include "catapult/utils/WrappedWithOwnerDecorator.h"
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	/// Wraps a strand and automatically augments handlers to extend the lifetime of an owning object.
	template<typename TOwner>
	class StrandOwnerLifetimeExtender {
	public:
		/// Creates a strand owner lifetime extender around \a strand.
		explicit StrandOwnerLifetimeExtender(boost::asio::strand& strand) : m_strand(strand)
		{}

	public:
		/// Wraps \a handler and attaches \a pOwner to it.
		template<typename THandler>
		auto wrap(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// when wrap is called the returned callback needs to extend the lifetime of pOwner
			utils::WrappedWithOwnerDecorator<THandler> wrappedHandler(std::move(handler), pOwner);
			return m_strand.wrap(wrappedHandler);
		}

	public:
		/// Attaches \a pOwner to \a handler and posts it to the strand.
		template<typename THandler>
		void post(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// ensure all handlers extend the lifetime of pOwner and post to a strand
			m_strand.post([pOwner, handler]() {
				handler(pOwner);
			});
		}

	private:
		boost::asio::strand& m_strand;
	};
}}
