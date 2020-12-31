//
// Created by Besogonov Aleksei on 12/26/20.
//

#include "logger.h"
#include "log_sink.h"
#include <fmt/format.h>
#include <charconv>

using namespace llog;

// Convert a reference-based attribute value to stored attribute value
struct saving_visitor {
	stored_attr_val_t val_;

	void operator()(const std::string_view &s) {
		val_ = std::move(std::string(s));
	}

	template<class T> void operator()(const T& val) {
		val_ = val;
	}
};


logger_t::logger_t(const logger_cfg_t &cfg, attribute_pack_t &&attrs) : cfg_(cfg) {
	for (auto &i : attrs) {
		saving_visitor sv;
		std::visit(sv, i.val_);
		attrs_.push_back(stored_attr_pair_t{
			.name_ = std::string(i.name_),
			.val_ = std::move(sv.val_),
		});
	}
}

struct arg_extracting_visitor {
	fmt::dynamic_format_arg_store<fmt::format_context> &args_;
	const char *name_;

	template<class T> void operator()(const T &val) {
		args_.push_back(fmt::arg(name_, val));
	}
};

struct push_formatter {
	sink_t &sink;
	void push_back(const char c) { sink.write(&c, 1); }
	typedef char value_type;
};

struct print_visitor {
	sink_t &sink_;

	template<class T> void operator ()(T val) {
		char buf[64];
		auto res = std::to_chars(buf, buf+sizeof(buf), val);
		if (!res.ptr) {
			throw std::bad_exception();
		}
		sink_.write(buf, res.ptr - buf);
	}
	void operator ()(double val) {
		char buf[64];
		auto ln = snprintf(buf, 64, "%f", val);
		sink_.write(buf, ln);
	}
	void operator ()(const std::string_view &val) {
		sink_.write(val.data(), val.size());
	}
	void operator ()(bool val) {
		if (val) {
			sink_.write("true", 4);
		} else {
			sink_.write("false", 5);
		}
	}
};

static std::string_view sev_to_text(severity_t sev) {
	switch (sev) {
		case TRACE:
			return "TRACE";
		case DEBUG:
			return "DEBUG";
		case INFO:
			return "INFO";
		case WARN:
			return "WARN";
		case ERROR:
			return "ERROR";
		case FATAL:
			return "FATAL";
	}
	return "UNKNWN";
}

void print_num(sink_t &sink, uint64_t num) {
	char buf[64];
	auto res = std::to_chars(buf, buf+sizeof(buf), num);
	if (!res.ptr) {
		throw std::bad_exception();
	}
	sink.write(buf, res.ptr - buf);
}

void logger_t::do_log(bool format, severity_t lvl,
											const std::string_view &msg, attribute_pack_t &&attrs) {

	sink_t &sink = *(cfg_.sink);
	sink_lock_guard_t lg(sink);

	// Format the string!
	if (cfg_.use_json) {
		if (cfg_.add_iso_timestamp) {
			sink.write("{\"ts\":", 6);
			sink.write("\"", 1);
			print_timestamp(cfg_.clock->now(), sink);
			sink.write("\",", 1);
		} else {
			sink.write("{\"millis\":", 10);
			timespec now = cfg_.clock->now();
			print_num(sink, now.tv_sec*1000L + now.tv_nsec / 1000000);
			sink.write(", \"nanos\":", 10);
			print_num(sink, now.tv_nsec % 1000000);
			sink.write(", ", 2);
		}
	} else {
		if (cfg_.add_iso_timestamp) {
			print_timestamp(cfg_.clock->now(), sink);
			sink.write("\t", 1);
		}

		const std::string_view &v = sev_to_text(lvl);
		sink.write(v.data(), v.size());
		sink.write("\t\"", 2);

		if (format) {
			// Format the log message
			fmt::dynamic_format_arg_store<fmt::format_context> store;
			for (auto &i : attrs) {
				arg_extracting_visitor v{store, i.name_};
				std::visit(v, i.val_);
			}

			push_formatter pf{.sink = sink};
			fmt::vformat_to(std::back_inserter(pf), msg, store);
			sink.write("\"\t", 2);
		} else {
			sink.write(msg.data(), msg.size());
			sink.write("\"\t", 2);
		}

		// Iterate over attrs and print them!
		for (auto &i : attrs) {
			sink.write("\t", 1);
			sink.write(i.name_, strlen(i.name_));
			sink.write("=", 1);
			print_visitor pv{.sink_ = sink};
			std::visit(pv, i.val_);
		}

		// If severity is above the backtrace threshold, then print it!
	}
}
