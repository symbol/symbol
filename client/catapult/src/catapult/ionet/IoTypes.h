#pragma once
#include <boost/asio.hpp>
#include <vector>
#include <stdint.h>

namespace catapult { namespace ionet {

	using ByteBuffer = std::vector<uint8_t>;

	using socket = boost::asio::ip::tcp::socket;
}}
