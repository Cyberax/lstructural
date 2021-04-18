//
// Created by Besogonov Aleksei on 12/25/20.
//
#pragma once

#include <string>
#include <variant>
#include <vector>
#include "clock.h"
#include "log_sink.h"

namespace llog {
	enum severity_t {
		TRACE = 10,
		DEBUG = 20,
		INFO = 30,
		WARN = 40,
		ERROR = 50,
		FATAL = 60
	};

	typedef std::variant<uint32_t, uint64_t, int32_t, int64_t, double,
		std::string_view, std::string, bool> attr_val_t;
	typedef std::variant<const char*, std::string> name_t;

	struct attr_pair_t {
		name_t name_;
		const attr_val_t val_;
	};
	typedef std::initializer_list<attr_pair_t> attribute_pack_t;


	inline attr_pair_t int32(const char *name, int32_t val) {
		return attr_pair_t{.name_ = name, .val_ = val};
	}
	inline attr_pair_t int32(std::string &&name, int32_t val) {
		return attr_pair_t{.name_ = std::move(name), .val_ = val};
	}

	inline attr_pair_t str(const char *name, const char* val) {
		return attr_pair_t{.name_ = name, .val_ = std::string_view(val)};
	}
	inline attr_pair_t str(std::string &&name, const char* val) {
		return attr_pair_t{.name_ = std::move(name), .val_ = std::string_view(val)};
	}

	inline attr_pair_t str(const char *name, std::string &&val) {
		return attr_pair_t{.name_ = name, .val_ = std::move(val)};
	}
	inline attr_pair_t str(std::string &&name, std::string &&val) {
		return attr_pair_t{.name_ = std::move(name), .val_ = std::move(val)};
	}

	struct logger_cfg_t {
		severity_t level = severity_t::INFO;
		severity_t backtrace_cutoff = severity_t::ERROR;

		bool add_iso_timestamp = true;
		const clock_t *clock = system_clock;

		bool use_json = false;
		std::shared_ptr<sink_t> sink = llog::cout_sink;
	};

//	inline namespace det {
//		typedef std::variant<uint32_t, uint64_t, int32_t, int64_t, double, std::string, bool> stored_attr_val_t;
//
//		struct stored_attr_pair_t {
//			const std::string name_;
//			const stored_attr_val_t val_;
//		};
//	}

	class logger_t
	{
		const logger_cfg_t &cfg_;
		std::vector<attr_pair_t> attrs_;
		sink_writer_t writer_;
	public:
		logger_t(const logger_cfg_t &cfg, attribute_pack_t &&attrs);

		bool should_log(severity_t lvl) const { return lvl >= cfg_.level; };

		template <class... Args>
		void logm(severity_t lvl, const std::string_view &msg, Args&& ...attrs) {
			auto lst = std::initializer_list<attr_pair_t> {attrs...};
			do_log(false, lvl, msg, std::forward<attribute_pack_t>(lst));
		}

		void log(severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs) {
			do_log(false, lvl, msg, std::forward<attribute_pack_t>(attrs));
		}
		void logf(severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs) {
			do_log(true, lvl, msg, std::forward<attribute_pack_t>(attrs));
		}

	private:
		void do_log(bool format, severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs);
	};

//#define LOG_INFO nanolog::is_logged(nanolog::LogLevel::INFO) && NANO_LOG(nanolog::LogLevel::INFO)
//#define LOG_WARN nanolog::is_logged(nanolog::LogLevel::WARN) && NANO_LOG(nanolog::LogLevel::WARN)
//#define LOG_CRIT nanolog::is_logged(nanolog::LogLevel::CRIT) && NANO_LOG(nanolog::LogLevel::CRIT)
}
