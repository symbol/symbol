#include "Logging.h"
#include "catapult/types.h"
#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/detail/default_attribute_names.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/phoenix.hpp>
#include <unordered_map>

#ifdef __clang__

namespace boost { namespace log { namespace sinks {
	extern template void basic_text_ostream_backend<char>::add_stream(const shared_ptr<stream_type>&);
}}}

#endif

namespace catapult { namespace utils {

	namespace {
		using SinkPointer = boost::shared_ptr<boost::log::sinks::sink>;
		using OverrideLevelsMap = std::vector<std::pair<const char*, LogLevel>>;

		// define attributes used in filtering
		BOOST_LOG_ATTRIBUTE_KEYWORD(subcomponent_tag, log::SubcomponentTraits::Name, log::SubcomponentTraits::Type);

		template<typename TTraits>
		void AddGlobalAttributeFromTraits(const typename TTraits::Type& defaultValue) {
			boost::log::core::get()->add_global_attribute(
					TTraits::Name,
					boost::log::attributes::constant<typename TTraits::Type>(defaultValue));
		}

		void InitializeGlobalLogAttributes() {
			namespace names = boost::log::aux::default_attribute_names;

			auto pCore = boost::log::core::get();
			pCore->add_global_attribute(names::timestamp(), boost::log::attributes::local_clock());
			pCore->add_global_attribute(names::thread_id(), boost::log::attributes::current_thread_id());
			AddGlobalAttributeFromTraits<log::FilenameTraits>("");
			AddGlobalAttributeFromTraits<log::SubcomponentTraits>(RawString());
			AddGlobalAttributeFromTraits<log::LineNumberTraits>(0);
		}

		template<typename TTraits>
		auto ExpressionFromTraits() {
			return boost::log::expressions::attr<typename TTraits::Type>(TTraits::Name);
		}

		enum Colors {
			Fg_Black = 30,
			Fg_Red = 31,
			Fg_Green = 32,
			Fg_Yellow = 33,
			Fg_Blue = 34,
			Fg_Magenta = 35,
			Fg_Cyan = 36,
			Fg_White = 37,
			Fg_Mask = 0x7f,
			Mode_Bright = 0x80
		};

		uint32_t ColorMappingFlags[] = {
			/* trace   */ 0,
			/* debug   */ 0,
			/* info    */ 0,
			/* warning */ Fg_Yellow,
			/* error   */ Fg_Red,
			/* fatal   */ Fg_Red
		};

		template<LogColorMode Mode>
		struct severity_color {
			static constexpr auto ColorMode = Mode;
		};

		template<LogColorMode Mode>
		void OutputAnsiCode(boost::log::formatting_ostream& stream, uint32_t flags) {
			auto foreground = flags & Fg_Mask;
			if (0 == foreground)
				return;

			// apply color
			stream << "\033[" << foreground << (LogColorMode::AnsiBold == Mode ? ";1" : "") << "m";
		}

		// The operator is used when putting the severity_level to log with a severity_color manipulator
		template<LogColorMode Mode>
		boost::log::formatting_ostream& operator<<(
				boost::log::formatting_ostream& stream,
				const boost::log::to_log_manip<boost::log::trivial::severity_level, severity_color<Mode>>& manipulator) {
			auto level = static_cast<std::size_t>(manipulator.get());
			if (level < CountOf(ColorMappingFlags) && ColorMappingFlags[level])
				OutputAnsiCode<Mode>(stream, ColorMappingFlags[level]);

			return stream;
		}

		std::string GetFormatSequence(LogColorMode colorMode) {
			// 2016-04-24 12:22:06.358231 0x00007fff774eb000: <info> (boot::Logging.cpp@106) msg
			std::string format("%1% %2%: <%3%> (%4%::%5%@%6%) %7%");
			return LogColorMode::None == colorMode
					? format
					: "%8%" + format + " \033[0m";
		}

		boost::log::formatter CreateLogFormatter(LogColorMode colorMode) {
			namespace expr = boost::log::expressions;
			namespace names = boost::log::aux::default_attribute_names;
			using boost::log::trivial::severity_level;

			auto formatter = expr::format(GetFormatSequence(colorMode))
					% expr::format_date_time<boost::posix_time::ptime>(names::timestamp(), "%Y-%m-%d %H:%M:%S.%f")
					% expr::attr<boost::log::attributes::current_thread_id::value_type>(names::thread_id())
					% expr::attr<severity_level>(names::severity())
					% ExpressionFromTraits<log::SubcomponentTraits>()
					% ExpressionFromTraits<log::FilenameTraits>()
					% ExpressionFromTraits<log::LineNumberTraits>()
					% expr::smessage;

			if (LogColorMode::Ansi == colorMode)
				return formatter % expr::attr<severity_level, severity_color<LogColorMode::Ansi>>(names::severity());

			return formatter % expr::attr<severity_level, severity_color<LogColorMode::AnsiBold>>(names::severity());
		}

		template<typename T>
		bool ShouldLog(T&& severity, LogLevel level) {
			return severity >= static_cast<boost::log::trivial::severity_level>(level);
		}

		boost::log::filter CreateLogFilter(LogLevel defaultLevel, const OverrideLevelsMap& overrideLevels) {
			return boost::phoenix::bind([defaultLevel, overrideLevels](const auto& severity, const auto& tag) {
				for (const auto& pair : overrideLevels) {
					// override level is set for this tag, so use it
					if (0 == strncmp(pair.first, tag->pData, tag->Size))
						return ShouldLog(severity, pair.second);
				}

				// no overrides set for this tag, so use the default level
				return ShouldLog(severity, defaultLevel);
			}, boost::log::trivial::severity.or_throw(), subcomponent_tag.or_throw());
		}
	}

	// region LogFilter

	// region Impl

	class LogFilter::Impl {
	public:
		boost::log::filter toBoostFilter() const {
			return CreateLogFilter(m_defaultLevel, m_overrideLevels);
		}

	public:
		void setLevel(LogLevel level) {
			m_defaultLevel = level;
		}

		void setLevel(const char* name, LogLevel level) {
			m_overrideLevels.emplace_back(name, level);
		}

	private:
		LogLevel m_defaultLevel;
		OverrideLevelsMap m_overrideLevels;
	};

	// endregion

	LogFilter::LogFilter(LogLevel level) : m_pImpl(std::make_unique<Impl>()) {
		m_pImpl->setLevel(level);
	}

	LogFilter::~LogFilter() = default;

	boost::log::filter LogFilter::toBoostFilter() const {
		return m_pImpl->toBoostFilter();
	}

	void LogFilter::setLevel(const char* name, LogLevel level) {
		m_pImpl->setLevel(name, level);
	}

	// endregion

	// region LoggingBootstrapper

	// region Impl

	class LoggingBootstrapper::Impl {
	public:
		~Impl() {
			// remove and flush all sinks
			for (const auto& pSink : m_sinks) {
				boost::log::core::get()->remove_sink(pSink);
				pSink->flush();
			}
		}

	public:
		template<typename TBackend>
		void addBackend(const boost::shared_ptr<TBackend>& pBackend, const BasicLoggerOptions& options, const LogFilter& filter) {
			using namespace boost::log::sinks;

			switch (options.SinkType) {
			case LogSinkType::Async:
				return addSink(boost::make_shared<asynchronous_sink<TBackend>>(pBackend), options.ColorMode, filter);

			default:
				return addSink(boost::make_shared<synchronous_sink<TBackend>>(pBackend), options.ColorMode, filter);
			}
		}

	private:
		template<typename TSinkPointer>
		void addSink(const TSinkPointer& pSink, LogColorMode colorMode, const LogFilter& filter) {
			pSink->set_filter(filter.toBoostFilter());
			pSink->set_formatter(CreateLogFormatter(colorMode));
			boost::log::core::get()->add_sink(pSink);
			m_sinks.push_back(pSink);
		}

	private:
		std::vector<SinkPointer> m_sinks;
	};

	// endregion

	LoggingBootstrapper::LoggingBootstrapper() : m_pImpl(std::make_unique<Impl>()) {
		InitializeGlobalLogAttributes();
	}

	LoggingBootstrapper::~LoggingBootstrapper() = default;

	void LoggingBootstrapper::addConsoleLogger(const BasicLoggerOptions& options, const LogFilter& filter) {
		using backend_t = boost::log::sinks::basic_text_ostream_backend<char>;
		auto pBackend = boost::make_shared<backend_t>();

		boost::shared_ptr<std::basic_ostream<char>> pStream(&std::clog, boost::null_deleter());
		pBackend->add_stream(pStream);

		m_pImpl->addBackend(pBackend, options, filter);
	}

	void LoggingBootstrapper::addFileLogger(const FileLoggerOptions& options, const LogFilter& filter) {
		namespace keywords = boost::log::keywords;
		using backend_t = boost::log::sinks::text_file_backend;
		auto pBackend = boost::make_shared<backend_t>(
				keywords::file_name = options.FilePattern,
				keywords::rotation_size = options.RotationSize,
				keywords::open_mode = std::ios_base::app);

		auto pCollector = boost::log::sinks::file::make_collector(
				keywords::target = options.Directory,
				keywords::max_size = options.MaxTotalSize,
				keywords::min_free_space = options.MinFreeSpace);
		pBackend->set_file_collector(pCollector);
		pBackend->scan_for_files(boost::log::sinks::file::scan_matching);

		m_pImpl->addBackend(pBackend, options, filter);
	}

	// endregion
}}
