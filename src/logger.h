//
// Created by Besogonov Aleksei on 12/25/20.
//
#pragma once

#include <string>
#include <variant>
#include <vector>
#include "clock.h"

namespace llog {
	class sink_t;
	extern std::shared_ptr<sink_t> cout_sink;

	enum severity_t {
		TRACE = 10,
		DEBUG = 20,
		INFO = 30,
		WARN = 40,
		ERROR = 50,
		FATAL = 60
	};

	typedef std::variant<uint32_t, uint64_t, int32_t, int64_t, double, std::string_view, bool> attr_val_t;

	struct attr_pair_t {
		const char *name_;
		const attr_val_t val_;
	};
	typedef std::initializer_list<attr_pair_t> attribute_pack_t;

	struct logger_cfg_t {
		severity_t level = severity_t::INFO;
		severity_t backtrace_cutoff = severity_t::ERROR;

		bool add_iso_timestamp = true;
		const clock_t *clock = system_clock;

		bool use_json = false;
		std::shared_ptr<sink_t> sink = llog::cout_sink;
	};

	inline namespace det {
		typedef std::variant<uint32_t, uint64_t, int32_t, int64_t, double, std::string, bool> stored_attr_val_t;

		struct stored_attr_pair_t {
			const std::string name_;
			const stored_attr_val_t val_;
		};
	}

	class logger_t
	{
		logger_cfg_t cfg_;
		std::vector<stored_attr_pair_t> attrs_;
	public:
		logger_t(logger_cfg_t &&cfg, attribute_pack_t &&attrs);

		bool should_log(severity_t lvl) const { return lvl >= cfg_.level; };

		void log(severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs);
		void logf(severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs);
	};

//#define LOG_INFO nanolog::is_logged(nanolog::LogLevel::INFO) && NANO_LOG(nanolog::LogLevel::INFO)
//#define LOG_WARN nanolog::is_logged(nanolog::LogLevel::WARN) && NANO_LOG(nanolog::LogLevel::WARN)
//#define LOG_CRIT nanolog::is_logged(nanolog::LogLevel::CRIT) && NANO_LOG(nanolog::LogLevel::CRIT)
}
