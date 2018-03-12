#include "ApiNodeHealthUtils.h"
#include "catapult/ionet/ConnectResult.h"
#include "catapult/ionet/Node.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/utils/ConfigurationValueParsers.h"
#include <boost/asio.hpp>
#include <regex>
#include <unordered_map>

namespace catapult { namespace tools { namespace health {

	namespace {
		// matcher for async_read_until that will match when at least one closing brace has been found
		// and all opening and closing braces are balanced
		struct BalancedBraceMatcher {
		private:
			using iterator = boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type>;

		public:
			BalancedBraceMatcher() : m_numUnmatchedOpenBraces(0)
			{}

		public:
			std::pair<iterator, bool> operator()(iterator begin, iterator end) {
				for (auto iter = begin; end != iter; ++iter) {
					switch (*iter) {
					case '{':
						++m_numUnmatchedOpenBraces;
						break;

					case '}':
						if (0 == --m_numUnmatchedOpenBraces)
							return std::make_pair(iter, true);

						break;
					}
				}

				return std::make_pair(end, false);
			}

		private:
			size_t m_numUnmatchedOpenBraces;
		};
	}
}}}

namespace boost { namespace asio {

	// specialization that allows async_read_until to use BalancedBraceMatcher as a matcher
	template<>
	struct is_match_condition<catapult::tools::health::BalancedBraceMatcher> : public boost::true_type
	{};
}}

namespace catapult { namespace tools { namespace health {

	namespace {
		constexpr uint16_t Rest_Api_Port = 3000;

		bool ShouldAbort(const boost::system::error_code& ec, const std::string& host, const char* operation) {
			if (!ec)
				return false;

			CATAPULT_LOG(error) << "failed when " << operation << " '" << host << "': " << ec.message();
			return true;
		}

		// region SocketConnector

		// similar to BasicConnectHandler in PacketSocket but does not require use of strands because it is not cancelable
		class SocketConnector final {
		private:
			using ResolverType = boost::asio::ip::tcp::resolver;

		public:
			SocketConnector(boost::asio::io_service& service, const std::string& host, uint16_t port)
					: m_socket(service)
					, m_resolver(service)
					, m_host(host + ":" + std::to_string(port))
					, m_query(host, std::to_string(port))
			{}

		public:
			thread::future<ionet::ConnectResult> future() {
				return m_promise.get_future();
			}

			boost::asio::ip::tcp::socket& socket() {
				return m_socket;
			}

			const std::string& host() const {
				return m_host;
			}

		public:
			void start() {
				m_resolver.async_resolve(m_query, [this](const auto& ec, auto iterator) {
					this->handleResolve(ec, iterator);
				});
			}

		private:
			void handleResolve(const boost::system::error_code& ec, const ResolverType::iterator& iterator) {
				if (ShouldAbort(ec, m_host, "resolving address"))
					return complete(ionet::ConnectResult::Resolve_Error);

				m_endpoint = iterator->endpoint();
				m_socket.async_connect(m_endpoint, [this](const auto& connectEc) {
					this->handleConnect(connectEc);
				});
			}

			void handleConnect(const boost::system::error_code& ec) {
				if (ShouldAbort(ec, m_host, "connecting to"))
					return complete(ionet::ConnectResult::Connect_Error);

				CATAPULT_LOG(info) << "connected to " << m_host << " [" << m_endpoint << "]";
				complete(ionet::ConnectResult::Connected);
			}

			void complete(ionet::ConnectResult result) {
				m_promise.set_value(std::move(result));
			}

		private:
			boost::asio::ip::tcp::socket m_socket;
			ResolverType m_resolver;

			std::string m_host;
			ResolverType::query m_query;

			boost::asio::ip::tcp::endpoint m_endpoint;
			thread::promise<ionet::ConnectResult> m_promise;
		};

		// endregion

		// region MultiHttpGetRetriever

		// makes multiple HTTP GET requests to a single node and processes simple JSON results
		class MultiHttpGetRetriever {
		public:
			using ResultType = std::unordered_map<std::string, uint64_t>;

			enum class Result {
				Connection_Error,
				Read_Error,
				Write_Error,
				Success
			};

		public:
			MultiHttpGetRetriever(
					boost::asio::io_service& service,
					const std::string& host,
					uint16_t port,
					const std::vector<std::string>& apiUris)
					: m_connector(service, host, port)
					, m_apiUris(apiUris)
			{}

		public:
			thread::future<ResultType> future() {
				return m_promise.get_future();
			}

			void start() {
				m_connector.start();
				m_connector.future().then([this](auto&& connectResult) {
					if (ionet::ConnectResult::Connected == connectResult.get())
						this->writeHttpGetRequest();
					else
						this->complete(Result::Connection_Error);
				});
			}

		private:
			void writeHttpGetRequest() {
				if (m_apiUris.empty())
					return complete(Result::Success);

				auto apiUri = m_apiUris.back();
				m_apiUris.pop_back();

				// create and send an HTTP GET request
				std::ostream requestStream(&m_request);
				requestStream
						<< "GET " << apiUri << " HTTP/1.1\r\n"
						<< "Host: " << m_connector.host() << "\r\n"
						<< "Accept: */*\r\n\r\n";

				boost::asio::async_write(m_connector.socket(), m_request, [this](const auto& ec, auto) {
					this->handleWriteHttpGetRequest(ec);
				});
			}

			void handleWriteHttpGetRequest(const boost::system::error_code& ec) {
				if (ShouldAbort(ec, m_connector.host(), "writing to"))
					return complete(Result::Write_Error);

				readHttpGetResponse();
			}

			void readHttpGetResponse() {
				boost::asio::async_read_until(m_connector.socket(), m_response, BalancedBraceMatcher(), [this](const auto& ec, auto) {
					this->handleReadHttpGetResponse(ec);
				});
			}

			void handleReadHttpGetResponse(const boost::system::error_code& ec) {
				if (ShouldAbort(ec, m_connector.host(), "reading from"))
					return complete(Result::Read_Error);

				// convert the response to a string and parse out values from it
				std::ostringstream out;
				out << &m_response;
				ParseResponseBody(out.str(), m_values);

				// chain the next HTTP GET request (write will complete successfully if none are left)
				writeHttpGetRequest();
			}

		private:
			void complete(Result result) {
				if (Result::Success == result) {
					m_promise.set_value(std::move(m_values));
					return;
				}

				std::ostringstream out;
				out << "failed processing data from " << m_connector.host() << " with error code " << utils::to_underlying_type(result);
				m_promise.set_exception(std::make_exception_ptr(catapult_runtime_error(out.str().c_str())));
			}

		private:
			static std::pair<std::string, uint64_t> ParseJsonUint64Value(const std::string& jsonPart) {
				// match: "height":[12,0]
				std::regex uint64ValueRegex("\"(\\w+)\":\\[(\\d+),(\\d+)\\]");
				std::smatch uint64ValueMatch;
				std::regex_match(jsonPart, uint64ValueMatch, uint64ValueRegex);

				// first part is the name (height)
				auto name = uint64ValueMatch[1];

				// next parts are the values low (12) and high (0)
				uint64_t value = 0;
				uint64_t parsedValue = 0;
				utils::TryParseValue(uint64ValueMatch[2], parsedValue);
				value = parsedValue;
				utils::TryParseValue(uint64ValueMatch[3], parsedValue);
				value |= (parsedValue << 32);

				return std::make_pair(name, value);
			}

			// dumb parser that only supports flat JSON documents composed entirely of uint64 values
			static void ParseResponseBody(const std::string& response, ResultType& values) {
				// match {(.*)}
				auto openingBraceIndex = response.find_first_of('{');
				auto closingBraceIndex = response.find_first_of('}', openingBraceIndex);
				auto jsonBodyWithoutBraces = response.substr(openingBraceIndex + 1, closingBraceIndex - openingBraceIndex - 1);

				// all uint64 values end with ']', so separator is effectively "],"
				std::vector<std::string> jsonParts;
				for (;;) {
					auto closingBracketIndex = jsonBodyWithoutBraces.find_first_of(']');
					jsonParts.push_back(jsonBodyWithoutBraces.substr(0, closingBracketIndex + 1));

					auto nextStartIndex = closingBracketIndex + 2; // skip "],"
					if (nextStartIndex >= jsonBodyWithoutBraces.size())
						break;

					jsonBodyWithoutBraces = jsonBodyWithoutBraces.substr(nextStartIndex);
				}

				for (const auto& jsonPart : jsonParts)
					values.emplace(ParseJsonUint64Value(jsonPart));
			}

		private:
			SocketConnector m_connector;
			std::vector<std::string> m_apiUris;

			boost::asio::streambuf m_request;
			boost::asio::streambuf m_response;

			ResultType m_values;
			thread::promise<ResultType> m_promise;
		};

		// endregion
	}

	thread::future<api::ChainInfo> CreateApiNodeChainInfoFuture(thread::IoServiceThreadPool& pool, const ionet::Node& node) {
		auto apiUris = std::vector<std::string>{ "/chain/height", "/chain/score" };
		auto pRetriever = std::make_shared<MultiHttpGetRetriever>(pool.service(), node.endpoint().Host, Rest_Api_Port, apiUris);
		pRetriever->start();
		return pRetriever->future().then([pRetriever](auto&& valuesMapFuture) {
			auto valuesMap = valuesMapFuture.get();
			api::ChainInfo chainInfo;
			chainInfo.Height = Height(valuesMap["height"]);
			chainInfo.Score = model::ChainScore(valuesMap["scoreHigh"], valuesMap["scoreLow"]);
			return chainInfo;
		});
	}
}}}
