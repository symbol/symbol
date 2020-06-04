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
#include "PathUtils.h"
#include <boost/log/attributes/constant.hpp>
#include <boost/log/core.hpp>
#include <boost/log/detail/light_rw_mutex.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/threading_models.hpp>
#include <boost/log/utility/strictest_lock.hpp>
#include <boost/thread/lock_guard.hpp>

namespace catapult { namespace utils {

	// region LogLevel

	/// Catapult log levels.
	/// \note For nicer CATAPULT_LOG construction, use severity_level names from boost::log::trivial.
	enum class LogLevel {
		/// Level for logging trace events.
		trace,

		/// Level for logging debug events.
		debug,

		/// Level for logging informational events.
		info,

		/// Level for logging important informational events.
		important,

		/// Level for logging warning events.
		warning,

		/// Level for logging error events.
		error,

		/// Level for logging fatal events.
		fatal,

		/// Minimum log level.
		min = trace,

		/// Maximum log level.
		max = fatal
	};

	/// Insertion operator for outputting \a level to \a out.
	std::ostream& operator<<(std::ostream& out, LogLevel level);

	// endregion

	// region LogSinkType

	/// Catapult log sink types.
	enum class LogSinkType {
		/// Synchronous sink.
		Sync,

		/// Asynchronous sink.
		Async
	};

	// endregion

	// region LogColorMode

	/// Catapult (console) log color modes.
	enum class LogColorMode {
		/// Ansi codes.
		Ansi,

		/// Bold ansi codes.
		AnsiBold,

		/// No coloring.
		None
	};

	// endregion

	// region LogFilter

	/// Filter used for filtering logs by level and/or component.
	struct LogFilter {
	public:
		/// Creates a log filter that sets the default log \a level across all components.
		explicit LogFilter(LogLevel level);

		/// Destroys the log filter.
		~LogFilter();

	public:
		/// Creates an equivalent boost log filter.
		boost::log::filter toBoostFilter() const;

	public:
		/// Sets the log \a level for the component specified by \a name.
		void setLevel(const char* name, LogLevel level);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};

	// endregion

	// region [Basic|File]LoggerOptions

	/// Basic logger options
	struct BasicLoggerOptions {
	public:
		/// Creates default options.
		BasicLoggerOptions()
				: SinkType(LogSinkType::Async)
				, ColorMode(LogColorMode::None)
		{}

		/// Log sink type.
		LogSinkType SinkType;

		/// Log color mode.
		LogColorMode ColorMode;
	};

	/// File logger options.
	struct FileLoggerOptions : public BasicLoggerOptions {
		/// Creates options that specify the creation of log files with the pattern \a filePattern in the \a directory.
		FileLoggerOptions(const std::string& directory, const std::string& filePattern)
				: Directory(directory)
				, FilePattern(filePattern)
		{}

		/// Log directory.
		std::string Directory;

		/// Log filename pattern.
		std::string FilePattern;

		/// File rotation size.
		uint64_t RotationSize = 25u * 1024 * 1024;

		/// Maximum size of all log files.
		uint64_t MaxTotalSize = 100u * 25 * 1024 * 1024;

		/// Minimum size of free disk space in order to create log files.
		uint64_t MinFreeSpace = 100u * 1024 * 1024;
	};

	// endregion

	// region LoggingBootstrapper

	/// Bootstraps boost logging.
	class LoggingBootstrapper final {
	public:
		/// Creates a bootstrapper.
		LoggingBootstrapper();

		/// Destroys the bootstrapper.
		~LoggingBootstrapper();

	public:
		/// Adds a console logger with the specified \a options and \a filter.
		void addConsoleLogger(const BasicLoggerOptions& options, const LogFilter& filter);

		/// Adds a file logger with the specified \a options and \a filter.
		void addFileLogger(const FileLoggerOptions& options, const LogFilter& filter);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};

	// endregion

	/// Flushes all buffered log records and waits for all sinks to complete processing of them.
	/// \note This function is only intended to be called right before a crash.
	void CatapultLogFlush();

	// region boost logging configuration and utils

	namespace log {
		/// Removes an item from an attributes collection on destruction.
		template<typename TAttributes>
		class EraseOnExit {
		private:
			using IteratorType = typename std::remove_reference_t<TAttributes>::iterator;

		public:
			EraseOnExit(TAttributes& attrs, IteratorType& iter) : m_attrs(attrs), m_iter(iter)
			{}

			~EraseOnExit() {
				if (m_iter != m_attrs.end())
					m_attrs.erase(m_iter);
			}

		private:
			TAttributes& m_attrs;
			IteratorType& m_iter;
		};

		/// Custom logging feature that allows tagging a log record with custom information.
		template<typename TBase, typename TTraits>
		class custom_info_tagger_feature : public TBase {
		public:
			using char_type = typename TBase::char_type;
			using threading_model = typename TBase::threading_model;

		public:
			custom_info_tagger_feature()
			{}

			custom_info_tagger_feature(const custom_info_tagger_feature& rhs) : TBase(static_cast<const TBase&>(rhs))
			{}

			template<typename TArgs>
			custom_info_tagger_feature(const TArgs& args) : TBase(args)
			{}

		public:
			// choose the most restrictive lock
			using open_record_lock = typename boost::log::strictest_lock<
				boost::lock_guard<threading_model>,
				typename TBase::open_record_lock,
				typename TBase::add_attribute_lock,
				typename TBase::remove_attribute_lock
			>::type;

		protected:
			template<typename TArgs>
			boost::log::record open_record_unlocked(const TArgs& args) {
				// extract value from parameter pack
				const auto& keyword = boost::parameter::keyword<typename TTraits::TagType>::instance;
				const auto& value = args[keyword];

				// add as a new attribute
				auto& attrs = TBase::attributes();
				auto result = TBase::add_attribute_unlocked(
						TTraits::Name,
						boost::log::attributes::constant<typename TTraits::Type>(value));
				auto iter = result.second ? result.first : attrs.end();

				// remove attribute upon scope exit
				EraseOnExit<decltype(attrs)> eraseGuard(attrs, iter);

				// forward to the base
				return TBase::open_record_unlocked(args);
			}
		};

		/// Allows custom_info_tagger_feature to be used as a source feature.
		template<typename TCustomFeatureTraits>
		struct custom_info_tagger {
			template<typename TBase>
			struct apply {
				using type = custom_info_tagger_feature<TBase, TCustomFeatureTraits>;
			};
		};

		/// Custom keywords that are used in associative arguments.
		namespace keywords {
			BOOST_PARAMETER_KEYWORD(tag, loglevel)
			BOOST_PARAMETER_KEYWORD(tag, line)
			BOOST_PARAMETER_KEYWORD(tag, file)
			BOOST_PARAMETER_KEYWORD(tag, subcomponent)
		}

		/// Traits for attaching a log level to a log record.
		struct LogLevelTraits {
			using Type = LogLevel;
			using TagType = keywords::tag::loglevel;
			static constexpr auto Name = "LogLevel";
		};

		/// Traits for attaching a line number to a log record.
		struct LineNumberTraits {
			using Type = unsigned int;
			using TagType = keywords::tag::line;
			static constexpr auto Name = "Line";
		};

		/// Traits for attaching a filename to a log record.
		struct FilenameTraits {
			using Type = const char*;
			using TagType = keywords::tag::file;
			static constexpr auto Name = "File";
		};

		/// Traits for attaching a subcomponent name to a log record.
		struct SubcomponentTraits {
			using Type = RawString;
			using TagType = keywords::tag::subcomponent;
			static constexpr auto Name = "Subcomponent";
		};

		/// Catapult logger type.
		class catapult_logger :
				public boost::log::sources::basic_composite_logger<
						char,
						catapult_logger,
						boost::log::sources::multi_thread_model<boost::log::aux::light_rw_mutex>,
						boost::log::sources::features<
								custom_info_tagger<LogLevelTraits>,
								custom_info_tagger<LineNumberTraits>,
								custom_info_tagger<FilenameTraits>,
								custom_info_tagger<SubcomponentTraits>>> {
			// generate forwarding constructors
			BOOST_LOG_FORWARD_LOGGER_MEMBERS_TEMPLATE(catapult_logger)
		};

		/// Make the catapult logger the global, default logger.
		BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(global_logger, catapult::utils::log::catapult_logger)
	}

	// endregion
}}

#define CATAPULT_LOG_WITH_LOGGER_LEVEL_TAG(LOGGER, LEVEL, TAG) \
	BOOST_LOG_STREAM_WITH_PARAMS( \
		(LOGGER), \
		(::catapult::utils::log::keywords::file = (::catapult::utils::ExtractFilename(__FILE__))) \
		(::catapult::utils::log::keywords::line = (static_cast<unsigned int>(__LINE__))) \
		(::catapult::utils::log::keywords::subcomponent = (TAG)) \
		(::catapult::utils::log::keywords::loglevel = (static_cast<::catapult::utils::LogLevel>(LEVEL))))

/// Writes a log entry to \a LOGGER with \a LEVEL severity.
#define CATAPULT_LOG_WITH_LOGGER_LEVEL(LOGGER, LEVEL) \
	CATAPULT_LOG_WITH_LOGGER_LEVEL_TAG(LOGGER, LEVEL, (::catapult::utils::ExtractDirectoryName(__FILE__)))

/// Writes a log entry to the default logger with \a LEVEL severity.
#define CATAPULT_LOG_LEVEL(LEVEL) \
	CATAPULT_LOG_WITH_LOGGER_LEVEL(::catapult::utils::log::global_logger::get(), LEVEL)

/// Writes a log entry to the default logger with \a SEV severity.
#define CATAPULT_LOG(SEV) \
	CATAPULT_LOG_WITH_LOGGER_LEVEL( \
			::catapult::utils::log::global_logger::get(), \
			(::catapult::utils::LogLevel::SEV))
